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
static void identifier(ParserState *parser);
static void grouping(ParserState *parser);
static void binary(ParserState *parser);
static void unary(ParserState *parser);
static void emit_return(ByteArray *code);

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
};

ParserState *parse(TokenArray *tokens, FunctionTable *functions) {
    ParserState *parser = init_parser(tokens, functions);

    // emit_stack_frame(parser->code, 0);
    // begin expr with 53 48 89 fb to push rdi and move to rbx
    ByteArray *code = parser->code;
    /*
    code->array[code->elements++] = 0x48;
    code->array[code->elements++] = 0x83;
    code->array[code->elements++] = 0xec;
    code->array[code->elements++] = 0x08;
    */
    parse_precedence(parser, PREC_NUMBER);
    /*
    code->array[code->elements++] = 0x48;
    code->array[code->elements++] = 0x83;
    code->array[code->elements++] = 0xc4;
    code->array[code->elements++] = 0x08;
    */
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
    } else {
        *(writer++) = 0xf3;
        *(writer++) = 0x0f;

        switch (instr) {
            case I_MOV:
                *(writer++) = 0x10;
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

/**
 * emits an operand to an instruction to fetch from memory
 */
void emit_mem_to_reg(ByteArray *code, unsigned int offset, unsigned int reg) {
    RESIZE_IF_NECESSARY(code, 5)

    uint8_t *writer = &code->array[code->elements];
    offset *= 4;
    if (offset == 0) { /* no offset */
        *(writer++) = 0x07 | reg << 3;
        code->elements++;
    } else if (offset < 128) { /* 1 byte offset */
        *(writer++) = 0x47 | reg << 3;
        *(writer++) = offset;
        code->elements += 2;
    } else { /* 4 byte offset in little-endian */
        *(writer++) = 0x87 | reg << 3;
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
        printf("x%02x(%%rdi),        %%xmm%d", code[4], (code[3] & 0x38) >> 3);
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
    case 0x53: printf("53                       push   %%rbx"); return 1;
    case 0x5b: printf("5b                       pop    %%rbx"); return 1;
    case 0xc3: printf("c3                       ret"); return 0;
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
        }
    case 0x66: 
        if (code[1] == 0x0f && code[2] == 0x3a && code[3] == 0x0a) {
            printf("66 0f 3a 0a %02x %02x        roundss", code[4], code[5]);
            return 6;
        }
    case 0x0f:
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
        emit_mem_to_reg(parser->code, parser->constants_ind++, parser->current_reg++);
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

static void identifier(ParserState *parser) {
    Token *var = parser->current;
    advance(parser);

    if (1 || parser->current_reg - 1 != 0) {
        emit_instruction(parser->code, I_MOV);
        emit_reg_to_reg(parser->code, 0, parser->current_reg++);
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

    if (!(operator->type == T_MOD) && !parser->parse_function && get_prev_move_dest(parser->code) == parser->current_reg - 1) {
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
        case T_DBLEQL: parser->error = 1; break;
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
        emit_mem_to_reg(parser->code, parser->constants_ind - 1, parser->current_reg - 2);
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
