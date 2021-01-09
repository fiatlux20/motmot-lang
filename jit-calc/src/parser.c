#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "lexer.h"
#include "table.h"
#include "function.h"
#include "parser.h"

static ParserState *init_parser(TokenArray *tokens, FunctionTable *functions);
static ByteArray *new_byte_array();
static void resize_byte_array(ByteArray *array, size_t new_size);
static void free_byte_array(ByteArray *array);

static void parse_precedence(ParserState *parser, Precedence precedence);
static void write_constants(ParserState *parser);
static void advance(ParserState *parser);
static Token *peek(ParserState *parser, unsigned int distance);
static void number(ParserState *parser);
static void function(ParserState *parser);
static void funccall(ParserState *parser);
static void if_stmt(ParserState *parser);
static void identifier(ParserState *parser);
static void grouping(ParserState *parser);
static void binary(ParserState *parser);
static void unary(ParserState *parser);
static void emit_return(ByteArray *code);
static void emit_function_header(ParserState *parser);
static void emit_function_footer(ParserState *parser);

typedef enum Register {
    RBP = 0,
    RDI = 2
} Register;

const static Rule rules[] = {
    [T_ERROR]   = { NULL,       NULL,   PREC_NONE },
    [T_END]     = { NULL,       NULL,   PREC_NONE },
    [T_NUMBER]  = { number,     NULL,   PREC_NUMBER },
    [T_VAR]     = { identifier, NULL,   PREC_NUMBER },
    [T_EQUALS]  = { NULL,       NULL,   PREC_NONE },
    [T_DBLEQL]  = { NULL,       binary, PREC_EQUALITY },
    [T_LPAREN]  = { grouping,   NULL,   PREC_NONE },
    [T_RPAREN]  = { NULL,       NULL,   PREC_NONE },
    [T_FUNC]    = { function,   NULL,   PREC_DEFUN },
    [T_FUNCALL] = { funccall,   NULL,   PREC_NUMBER },
    [T_PLUS]    = { NULL,       binary, PREC_TERM },
    [T_MINUS]   = { unary,      binary, PREC_TERM },
    [T_STAR]    = { NULL,       binary, PREC_FACTOR },
    [T_SLASH]   = { NULL,       binary, PREC_FACTOR },
    [T_MOD]     = { NULL,       binary, PREC_FACTOR },
    [T_SQRT]    = { unary,      NULL,   PREC_CALL },
    [T_IF]      = { if_stmt,    NULL,   PREC_CALL },
    [T_THEN]    = { NULL,       NULL,   PREC_NONE },
    [T_ELSE]    = { NULL,       NULL,   PREC_NONE },
};

ParserState *parse(TokenArray *tokens, FunctionTable *functions) {
    ParserState *parser = init_parser(tokens, functions);

    ByteArray *code = parser->code;

    if (tokens->func_name != NULL) {
        parser->parse_function = 1;

        emit_function_header(parser);
    }

    parse_precedence(parser, PREC_NUMBER);

    if (parser->parse_function) {
        emit_function_footer(parser);
    }

    emit_return(parser->code);

    if (parser->parse_function) {
        write_constants(parser);
        /* align chunk to 16 bytes */
        // while ((parser->code->elements & 15) != 0) {
        //     parser->code->elements++;
        // }
    }

    return parser;
}

void free_parser(ParserState *parser) {
    free(parser->constants);
    free(parser->flags);
    free_byte_array(parser->code);
    free(parser);
    parser = NULL;
}

/* */
static ParserState *init_parser(TokenArray *tokens, FunctionTable *functions) {
    ParserState *parser = malloc(sizeof *parser);
    parser->constants = malloc(sizeof *parser->constants * tokens->num_constants);
    parser->flags = malloc(sizeof *parser->flags * tokens->num_constants);
    parser->functions = functions;
    parser->current = tokens->tokens;
    parser->code = new_byte_array();
    parser->constants_ind = 0;
    parser->current_reg = 0;
    parser->error = 0;
    parser->parse_function = 0;
    return parser;
}

static ByteArray *new_byte_array() {
    ByteArray *array = malloc(sizeof *array);
    array->capacity = 256;
    array->elements = 0;
    array->array = malloc(sizeof *array->array * array->capacity);
    return array;
}

static void resize_byte_array(ByteArray *array, size_t new_size) {
    uint8_t *old = array->array;
    array->array = malloc((sizeof *array->array) * new_size);
    array->capacity = new_size;

    memcpy(array->array, old, array->elements);
    free(old);
}

static void free_byte_array(ByteArray *array) {
    free(array->array);
    free(array);
    array = NULL;
}

#define RESIZE_IF_NECESSARY(code, delta)             \
    if (code->capacity - code->elements < delta) {   \
        resize_byte_array(code, code->elements * 2); \
    }

/* -- Architecture specific functions ------------------------------------ */
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
/**
 * Functions for x64 CPUs
 */

/*
 * emit the first 3 bytes of an instruction without an operand
 */
void emit_instruction(ByteArray *code, Instruction instr) {
    RESIZE_IF_NECESSARY(code, 4)

    uint8_t *writer = &code->array[code->elements];

    if (instr == I_ROUND) {
        *(writer++) = 0x66;
        *(writer++) = 0x0f;
        *(writer++) = 0x3a;
        *(writer++) = 0x0a;
        code->elements += 4;
    } else if (instr == I_XOR) {
        *(writer++) = 0x0f;
        *(writer++) = 0x57;
        code->elements += 2;
    } else if (instr == I_COMISS) {
        *(writer++) = 0x0f;
        *(writer++) = 0x2e;
        code->elements += 2;
    } else if (instr == I_AND) {
        *(writer++) = 0x0f;
        *(writer++) = 0x54;
        code->elements += 2;
    } else {
        *(writer++) = 0xf3;
        *(writer++) = 0x0f;

        switch (instr) {
            case I_MOV:
                *(writer++) = 0x10;
                break;
            case I_MOV_STACK:
                *(writer++) = 0x11;
                break;
            case I_MOVAPS: break;
            case I_ADD:
                *(writer++) = 0x58;
                break;
            case I_SUB:
                *(writer++) = 0x5c;
                break;
            case I_MUL:
                *(writer++) = 0x59;
                break;
            case I_DIV:
                *(writer++) = 0x5e;
                break;
            case I_SQRT:
                *(writer++) = 0x51;
                break;
            case I_XOR:
                break;
            case I_CMP:
                *(writer++) = 0xc2;
                break;
        }
        code->elements += 3;
    }
}

/**
 * emits a relative call to 'position' in memory by calculating the offset
 * (https://www.felixcloutier.com/x86/call)
 */
void emit_rel_call(ParserState *parser, unsigned int position) {
    RESIZE_IF_NECESSARY(parser->code, 5)

    int offset = position - parser->functions->bytes_occupied - (parser->code->elements + 5);

    uint8_t *writer = &parser->code->array[parser->code->elements];

    *(writer++) = 0xe8;
    /* convert offset to little-endian */
    *(writer++) = (offset & 0xff);
    *(writer++) = (offset & 0xff00) >> 8;
    *(writer++) = (offset & 0xff0000) >> 16;
    *(writer++) = offset >> 24;

    parser->code->elements += 5;
}

void emit_reg_to_mem(ByteArray *code, int offset, unsigned int src, Register dst) {
    RESIZE_IF_NECESSARY(code, 5)

    uint8_t *writer = &code->array[code->elements];
    offset *= 4;
    if (offset >= -128 && offset <= 127) { /* 1 byte offset */
        *(writer++) = (0x45 + dst) | src << 3;
        *(writer++) = offset;
        code->elements += 2;
    } else { /* 4 byte offset in little-endian */
        *(writer++) = 0x87 | src << 3;
        *(writer++) = (offset & 0xff);
        *(writer++) = (offset & 0xff00) >> 8;
        *(writer++) = (offset & 0xff0000) >> 16;
        *(writer++) = offset >> 24;
        code->elements += 5;
    }
}
/**
 * emits an operand to an instruction to fetch from memory
 */
void emit_mem_to_reg(ByteArray *code, int offset, Register src, unsigned int dst) {
    RESIZE_IF_NECESSARY(code, 5)

    uint8_t *writer = &code->array[code->elements];
    offset *= 4;
    if (offset == 0) { /* no offset */
        *(writer++) = (0x05 + src) | dst << 3;
        code->elements++;
    } else if (offset >= -128 && offset <= 127) { /* 1 byte offset */
        *(writer++) = (0x45 + src) | dst << 3;
        *(writer++) = offset;
        code->elements += 2;
    } else { /* 4 byte offset in little-endian */
        *(writer++) = (0x85 + src) | dst << 3;
        *(writer++) = (offset & 0xff);
        *(writer++) = (offset & 0xff00) >> 8;
        *(writer++) = (offset & 0xff0000) >> 16;
        *(writer++) = offset >> 24;
        code->elements += 5;
    }
}

/**
 * emits a register to register operand to an instruction
 */
static void emit_reg_to_reg(ByteArray *code, unsigned int src, unsigned int dest) {
    RESIZE_IF_NECESSARY(code, 1)

    code->array[code->elements++] = 0xc0 | dest <<3 | src;
}

/**
 * emits a constant to register operation to transfer a constant defined in the code to a register
 * emits the operands offset(%rip), %xmm*
 */
static void emit_const_to_reg_abs(ParserState *parser, unsigned int position, unsigned int dest) {
    RESIZE_IF_NECESSARY(parser->code, 5);

    int offset = position - (parser->functions->bytes_occupied + parser->code->elements + 5);

    uint8_t *writer = &parser->code->array[parser->code->elements];
    *(writer++) = 0x05 | dest << 3;
    *(writer++) = (offset & 0xff);
    *(writer++) = (offset & 0xff00) >> 8;
    *(writer++) = (offset & 0xff0000) >> 16;
    *(writer++) = offset >> 24;
    parser->code->elements += 5;
}

static void emit_const_to_reg(ParserState *parser, unsigned int position, unsigned int dest) {
    RESIZE_IF_NECESSARY(parser->code, 5);

    int offset = position - (parser->code->elements + 5);

    uint8_t *writer = &parser->code->array[parser->code->elements];
    *(writer++) = 0x05 | dest << 3;
    *(writer++) = (offset & 0xff);
    *(writer++) = (offset & 0xff00) >> 8;
    *(writer++) = (offset & 0xff0000) >> 16;
    *(writer++) = offset >> 24;
    parser->code->elements += 5;
}

static void emit_return(ByteArray *code) {
    RESIZE_IF_NECESSARY(code, 1)

    code->array[code->elements++] = 0xc3;
}

static void emit_leave(ByteArray *code) {
    RESIZE_IF_NECESSARY(code, 1)

    code->array[code->elements++] = 0xc9;
}

static void write_float(ByteArray *code, float x) {
    RESIZE_IF_NECESSARY(code, 4)

    uint8_t *fp = (uint8_t*) &x;
    uint8_t *writer = &code->array[code->elements];
    *(writer++) = fp[0];
    *(writer++) = fp[1];
    *(writer++) = fp[2];
    *(writer++) = fp[3];
    code->elements += 4;
}

static void emit_byte(ByteArray *code, unsigned char byte) {
    RESIZE_IF_NECESSARY(code, 1)

    code->array[code->elements++] = byte;
}

static void emit_bytes(ByteArray *code, char *bytes, unsigned int length) {
    RESIZE_IF_NECESSARY(code, length);

    for (int i = 0; i < length; i++) {
        code->array[code->elements++] = bytes[i];
    }
}

static int emit_jmp(ByteArray *code, unsigned char offset) {
    RESIZE_IF_NECESSARY(code, 2)

    code->array[code->elements++] = 0xeb;
    code->array[code->elements++] = offset;
    return code->elements - 1;
}

static int emit_jne(ByteArray *code, unsigned char offset) {
    RESIZE_IF_NECESSARY(code, 2)

    code->array[code->elements++] = 0x75;
    code->array[code->elements++] = offset;
    return code->elements - 1;
}

static void emit_function_header(ParserState *parser) {
    // pushq %rbp
    // movq  %rsp, %rbp   - 55 48 89 e5
    emit_bytes(parser->code, "\x55\x48\x89\xe5", 4);

    // sub 4 rsp
    emit_bytes(parser->code, "\x48\x83\xec\x04", 4);

    // movss %xmm0, -4(%rbp)
    emit_instruction(parser->code, I_MOV_STACK);
    emit_reg_to_mem(parser->code, -1, 0, RBP);
}

static void emit_function_footer(ParserState *parser) {
    // leave
    emit_byte(parser->code, 0xc9);
}

static int get_prev_move_dest(ByteArray *code) {
    for (int i = 0; i < 5; i++) {
        if (code->array[code->elements - i] == 0x10 && code->array[code->elements - i - 1] == 0x0f && code->array[code->elements - i - 2] == 0xf3) {
            return (code->array[code->elements - i + 1] & 0x38) >> 3;
        }
    }
    return -1;
}

static int get_prev_move_offset(ByteArray *code) {
    for (int i = 0; i < 5; i++) {
        if (code->array[code->elements - i] == 0x10 && code->array[code->elements - i - 1] == 0x0f && code->array[code->elements - i - 2] == 0xf3) {
            return i + 2;
        }
    }
    return 0;
}

static int disassemble_fp_instr(unsigned char *code) {
    switch(code[3] & 0xc0) { /* get mode of instruction and print bytes*/
    case 0x00: /* mem to reg, no offset */
        if ((code[3] & 0x07) == 0x05) {
            printf("%02x %02x %02x %02x %02x %02x %02x %02x  ",
                    code[0], code[1], code[2], code[3], code[4],
                    code[5], code[6], code[7]);
        } else {
            printf("%02x %02x %02x %02x              ",
                    code[0], code[1], code[2], code[3]);
        }

        break;
    case 0x40: /* mem to reg, 1 byte offset */
        printf("%02x %02x %02x %02x %02x           ",
                code[0], code[1], code[2], code[3], code[4]);
        break;
    case 0x80: /* mem to reg, 4 byte offset */
        printf("%02x %02x %02x %02x %02x %02x %02x %02x  ",
                code[0], code[1], code[2], code[3], code[4],
                code[5], code[6], code[7]);
        break;
    case 0xc0: /* reg to reg */
        printf("%02x %02x %02x %02x              ",
                code[0], code[1], code[2], code[3]);
        break;
    }

    switch(code[2]) {
    case 0x10:
        printf("movss  ");
        break;
    case 0x11: /* move to/from rbp */
        printf("movss  ");
        break;
    case 0x58:
        printf("addss  ");
        break;
    case 0x57:
        printf("xorps  ");
        break;
    case 0x5c:
        printf("subss  ");
        break;
    case 0x59:
        printf("mulss  ");
        break;
    case 0x5e:
        printf("divss  ");
        break;
    case 0x51:
        printf("sqrtss ");
        break;
    case 0xc2:
        printf("cmpeqss ");
        break;
    case 0x2e:
        printf("ucomiss ");
        break;
    }

    /* check mode of operand byte */
    switch (code[3] & 0xc0) {
    case 0x00: /* mem to reg, no offset*/
        if ((code[3] & 0x07) == 0x05) {
            uint32_t offset = code[4] | code[5] << 8 | code[6] << 16 | code[7] << 24;
            printf("%3d(%%rip),        %%xmm%d", offset, (code[3] & 0x38) >> 3);
            return 8;
        } else {
            printf("(%%rdi),           %%xmm%d", (code[3] & 0x38) >> 3);
        }
        return 4;
    case 0x40: /* mem to reg, 1 byte offset */
        if ((code[3] & 0x07) == 0x05) {
            printf("%02d(%%rbp),        %%xmm%d", (signed char) code[4], (code[3] & 0x38) >> 3);
        } else {
            printf("x%02x(%%rdi),        %%xmm%d", code[4], (code[3] & 0x38) >> 3);
        }
        return 5;
    case 0x80: /* mem to reg, 4 byte offset */
        /* only prints between 0x00 and 0xff */
        if (code[5] == 0x00 && code[6] == 0x00 && code[7] == 0x00) {
            printf("x%02x(%%rdi),        %%xmm%d", code[4], (code[3] & 0x38) >> 3);
        } else {
            printf("x%02x%02x%02x%02x(%%rdi),  %%xmm%d", code[7], code[6],
                    code[5], code[4], (code[3] & 0x38) >> 3);
        }
        return 8;
        break;
    case 0xc0: /* reg to reg */
        printf("%%xmm%d,            %%xmm%d", code[3] & 0x07, (code[3] & 0x38) >> 3);
        return 4;
    }
}

/*
 *  disassembles the instruction beginning at *code
 */
static int disassemble_instr(unsigned char *code) {
    uint32_t offset;
    switch (code[0]) {
    case 0x55: printf("55                       push   %%rbp"); return 1;
    case 0x5d: printf("5d                       pop    %%rbp"); return 1;
    case 0xc3: printf("c3                       ret"); return 0;
    case 0xc9: printf("c9                       leave"); return 0;
    case 0xeb: printf("eb %02x                    jmp    %02d", code[1], code[1]); return 2;
    case 0x75: printf("75 %02x                    jne    %02d", code[1], code[1]); return 2;
    case 0xe8:
        offset = code[1] | code[2] << 8 | code[3] << 16 | code[4] << 24;
        printf("e8 %02x %02x %02x %02x           call   %d", code[1], code[2], code[3], code[4], (int32_t) offset);
        return 5;
    case 0x48:
        if (code[1] == 0x89 && code[2] == 0xfb) {
            printf("48 89 fb                 mov    %%rdi,             %%rbx");
            return 3;
        } else if (code[1] == 0x83 && code[2] == 0xec) {
            printf("48 83 ec %02x              sub    %d,                %%rsp", code[3], code[3]);
            return 4;
        } else if (code[1] == 0x83 && code[2] == 0xc4) {
            printf("48 83 c4 %02x              add    %d,                %%rsp", code[3], code[3]);
            return 4;
        } else if (code[1] == 0x89 && code[2] == 0xe5) {
            printf("48 89 e5                 mov    %%rsp,             %%rbp", code[3], code[3]);
            return 3;
        }
    case 0x66: 
        if (code[1] == 0x0f && code[2] == 0x3a && code[3] == 0x0a) {
            printf("66 0f 3a 0a %02x %02x        roundss %d", code[4], code[5], code[5]);
            return 6;
        }
    case 0x0f: return disassemble_fp_instr(code - 1) - 1;
    case 0xf3: return disassemble_fp_instr(code);
    default: printf("%02x                   unknown", code[0]); return 1;
    }
}

void disassemble(ByteArray *code) {
    printf("----- disassembly -----\n");
    unsigned char *reader = code->array;
    unsigned int offset = disassemble_instr(reader);
    printf("\n");
    while (offset != 0) {
        reader += offset;
        offset = disassemble_instr(reader);
        printf("\n");
        if (reader >= code->array + code->elements) {
            break;
        }
    }
}

/*
#elif defined(__aarch64__)
  #include "arm.h"
*/
#else
  #error "CPU architecture is unsupported."
#endif

static void swap_registers(ParserState *parser, unsigned int reg1, unsigned int reg2) {
    if (reg1 == reg2) {
        return;
    }

    emit_instruction(parser->code, I_MOV);
    emit_reg_to_reg(parser->code, reg1, parser->current_reg + 1);
    emit_instruction(parser->code, I_MOV);
    emit_reg_to_reg(parser->code, reg2, reg1);
    emit_instruction(parser->code, I_MOV);
    emit_reg_to_reg(parser->code, parser->current_reg + 1, reg2);
}

static const Rule *get_rule(Token *t) {
    return &rules[t->type];
}

static inline void advance(ParserState *parser) {
    parser->current++;
}

static Token *peek(ParserState *parser, unsigned int distance) {
    for (unsigned int i = 0; i < distance; i++) {
        if ((parser->current + i)->type == T_END) {
            return NULL;
        }
    }

    return parser->current + distance;
}

static int expect(ParserState *parser, TokenType type) {
    if (parser->current->type == type) {
        advance(parser);
        return 1;
    } else {
        parser->error = 1;
        return 0;
    }
}

static void number(ParserState *parser) {
    parser->constants[parser->constants_ind] = parser->current->value;
    if (parser->parse_function) {
        // flag this position to insert a move constant instruction later
        parser->flags[parser->constants_ind++] = parser->code->elements;
        emit_byte(parser->code, parser->current_reg++); // store register to use
        for (int i = 0; i < 7; i++) {
            emit_byte(parser->code, 0x90);
        }
    } else {
        emit_instruction(parser->code, I_MOV);
        emit_mem_to_reg(parser->code, parser->constants_ind++, RDI, parser->current_reg++);
    }
    advance(parser);
}

static void unary(ParserState *parser) {
    Token *operator = parser->current;
    advance(parser);

    parse_precedence(parser, get_rule(operator)->precedence + 1);

    switch (operator->type) {
        case T_MINUS:
            emit_instruction(parser->code, I_MOV);
            emit_const_to_reg_abs(parser, parser->functions->sign_offset, parser->current_reg);
            emit_instruction(parser->code, I_XOR);
            emit_reg_to_reg(parser->code, parser->current_reg, parser->current_reg - 1);
            break;
        case T_SQRT:
            emit_instruction(parser->code, I_SQRT);
            emit_reg_to_reg(parser->code, parser->current_reg - 1, parser->current_reg - 1);
            break;
        default:
            break;
    }
}

static void function(ParserState *parser) {
    parser->parse_function = 1;

    add_entry(parser->functions->table, parser->current->string, make_function(NULL, parser->functions->bytes_occupied));
    advance(parser);

    if (!expect(parser, T_LPAREN)) {
        report_error("SyntaxError", "Expected '(' after function name.");
    }

    Token *var = parser->current;
    if (var->type != T_VAR) {
        if (var->type == T_LPAREN) {
            report_error("SyntaxError", "Functions must take one argument.");
        } else {
            report_error("SyntaxError", "Expected argument name in function definition.");
        }
        parser->error = 1;
    }

    advance(parser);

    if (!expect(parser, T_RPAREN)) {
        if (parser->current->type == T_VAR) {
            report_error("SyntaxError", "Functions must take only one argument.");
        } else {
            report_error("SyntaxError", "Expected ')' closing function argument definition.");
        }
    }

    if (!expect(parser, T_EQUALS)) {
        report_error("SyntaxError", "Expected '=' after function definition.");
    }

    parse_precedence(parser, PREC_NUMBER);
}

static void funccall(ParserState *parser) {
    char *func_name = parser->current->string;
    Function *func = get_entry_value(parser->functions->table, func_name);
    if (func == NULL) {
        report_error("RuntimeError", "Function %s not found.", func_name);
    }

    advance(parser);
    
    if (!expect(parser, T_LPAREN)) {
        if (parser->current->type == T_NUMBER) {
            report_error("SyntaxError", "Function argument must be surrounded with parentheses.");
        } else {
            report_error("SyntaxError", "Expected '(' after function call.");
        }
    }

    parse_precedence(parser, PREC_NUMBER);

    swap_registers(parser, 0, --parser->current_reg);
    emit_rel_call(parser, func->offset);
    // swap_registers(parser, 0, parser->current_reg);
    parser->current_reg++;

    // advance(parser);

    if (!expect(parser, T_RPAREN)) {
        report_error("SyntaxError", "Expected ')' after function call.");
    }
}

static void if_stmt(ParserState *parser) {
    advance(parser);
    parse_precedence(parser, PREC_NUMBER);

    if (!expect(parser, T_THEN)) {
        report_error("SyntaxError", "Expected 'then' after 'if' expression.");
        return;
    }

    emit_instruction(parser->code, I_COMISS);
    emit_const_to_reg_abs(parser, parser->functions->one_offset, parser->current_reg - 1);
    int jne_offset = emit_jne(parser->code, 0);
    int jne_next_byte = parser->code->elements;
    parser->current_reg--;

    int current_reg = parser->current_reg;
    parse_precedence(parser, PREC_NUMBER);


    if (!expect(parser, T_ELSE)) {
        report_error("SyntaxError", "Expected 'else' after 'then'.");
        return;
    }

    parser->current_reg = current_reg;
    int jmp_offset = emit_jmp(parser->code, 0);
    int jmp_next_byte = parser->code->elements;

    // patch in jne offset
    parser->code->array[jne_offset] = parser->code->elements - jne_next_byte;

    parse_precedence(parser, PREC_NUMBER);

    // patch in jmp offset
    parser->code->array[jmp_offset] = parser->code->elements - jmp_next_byte;
}

static void identifier(ParserState *parser) {
    Token *var = parser->current;
    advance(parser);

    if (1 || parser->current_reg - 1 != 0) {
        emit_instruction(parser->code, I_MOV);
        emit_mem_to_reg(parser->code, -1, RBP, parser->current_reg++);
    }
}

static void binary(ParserState *parser) {
    Token *operator = parser->current;
    advance(parser);

    parse_precedence(parser, get_rule(operator)->precedence + 1);

    /* if the src register of this instruction is the same as the
     * destination register of the previous, e.g.
     *     movss  $04(%rdi),  %xmm1
     *     addss  %xmm1,      %xmm0
     *     (%xmm1 is the dest of movss, and src of addss)
     * then these instructions can be 'concatenated' to 
     *     addss  $04(%rdi),  %xmm0
     * which avoids using an extra register for simple expressions
     */
    int concatenating = 0;

    if (!(operator->type == T_MOD || operator->type == T_DBLEQL) && !parser->parse_function && get_prev_move_dest(parser->code) == parser->current_reg - 1) {
        concatenating = 1;
        int offset = get_prev_move_offset(parser->code);
        if (offset == 0) {
            concatenating = 0;
        } else {
            /* write over previous move instruction */
            parser->code->elements -= get_prev_move_offset(parser->code);
        }
    }

    switch (operator->type) {
        case T_PLUS:  emit_instruction(parser->code, I_ADD); break;
        case T_MINUS: emit_instruction(parser->code, I_SUB); break;
        case T_STAR:  emit_instruction(parser->code, I_MUL); break;
        case T_SLASH: emit_instruction(parser->code, I_DIV); break;
        case T_DBLEQL:
            emit_instruction(parser->code, I_CMP);
            emit_reg_to_reg(parser->code, parser->current_reg - 1, parser->current_reg - 2);
            emit_byte(parser->code, 0x00);
            emit_instruction(parser->code, I_MOV);
            emit_const_to_reg_abs(parser, parser->functions->one_offset, parser->current_reg - 1);
            emit_instruction(parser->code, I_AND);
            emit_reg_to_reg(parser->code, parser->current_reg - 1, parser->current_reg - 2);
            parser->current_reg--;
            return;
        case T_MOD: 
            /* simple mod: value - floor(value / mod) * mod
             *  movss   %xmm0, %xmm2
             *  movss   %xmm1, %xmm3
             *  divss   %xmm3, %xmm2
             *  roundss $1, %xmm2, %xmm3
             *  mulss   %xmm2, %xmm1
             *  subss   %xmm1, %xmm0
             */
            emit_instruction(parser->code, I_MOV);
            emit_reg_to_reg(parser->code, parser->current_reg - 2, parser->current_reg);
            emit_instruction(parser->code, I_MOV);
            emit_reg_to_reg(parser->code, parser->current_reg - 1, parser->current_reg + 1);
            emit_instruction(parser->code, I_DIV);
            emit_reg_to_reg(parser->code, parser->current_reg + 1, parser->current_reg);
            emit_instruction(parser->code, I_ROUND);
            emit_reg_to_reg(parser->code, parser->current_reg, parser->current_reg);
            emit_byte(parser->code, 0x01);
            emit_instruction(parser->code, I_MUL);
            emit_reg_to_reg(parser->code, parser->current_reg, parser->current_reg - 1);
            emit_instruction(parser->code, I_SUB);
            emit_reg_to_reg(parser->code, parser->current_reg - 1, parser->current_reg - 2);
            parser->current_reg--;
            return;

        default: break;
    }

    if (concatenating) {
        emit_mem_to_reg(parser->code, parser->constants_ind - 1, RDI, parser->current_reg - 2);
    } else {
        emit_reg_to_reg(parser->code, parser->current_reg - 1, parser->current_reg - 2);
    }

    parser->current_reg--;
}

static void grouping(ParserState *parser) {
    advance(parser);

    parse_precedence(parser, PREC_NUMBER);

    if (!expect(parser, T_RPAREN)) {
        report_error("SyntaxError", "Expected closing parenthesis ')'.");
    }
}

static void parse_precedence(ParserState *parser, Precedence precedence) {
    ParseFn prefix = get_rule(parser->current)->prefix;

    if (prefix == NULL) {
        report_error("SyntaxError", "Expected expression.");
        parser->error = 1;
        return;
    }

    prefix(parser);

    while (precedence <= get_rule(parser->current)->precedence) {
        ParseFn infix = get_rule(parser->current)->infix;
        infix(parser);
    }
}

static void write_constants(ParserState *parser) {
    for (int i = 0; i < parser->constants_ind; i++) {
        unsigned int position = parser->code->elements;
        // write constant
        write_float(parser->code, parser->constants[i]);
        // connect constant
        parser->code->elements = parser->flags[i];
        unsigned int reg = parser->code->array[parser->flags[i]];
        emit_instruction(parser->code, I_MOV);
        emit_const_to_reg(parser, position, reg);
        parser->code->elements = position + 4;
    }
}
