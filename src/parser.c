#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/* private functions */
static void assignment(ParserState*);
static void binary(ParserState*);
static void expression(ParserState*);
static void grouping(ParserState*);
static void identifier(ParserState*);
static void number(ParserState*);
static void string(ParserState*);
static void unary(ParserState*);

BytecodeArray *parse(VirtualMachine *vm, TokenArray *tokens) {
    ArrayIterator iter = { tokens->count, 0 };
    BytecodeArray *bytecode = create_bytecode_dynarray();
    bytecode->names = &vm->names;

    ParserState s;
    s.current = tokens->tokens;
    s.prev = NULL;
    s.bytecode = bytecode;
    s.iter = &iter;
    s.error = 0;

    expression(&s);
    // statement(&s);

    #ifdef DEBUG_PARSER
    printf("index at %ld out of %d\n", s.current - tokens->tokens, tokens->count);
    #endif

    return bytecode;
}

/* private functions */
const static Rule rules[] = {
    [T_NONE]         = { NULL,       NULL,   PREC_NONE },
    [T_EOF]          = { NULL,       NULL,   PREC_NONE },
    [T_ERROR]        = { NULL,       NULL,   PREC_NONE },
    [T_NIL]          = { NULL,       NULL,   PREC_NONE },
    [T_IDENTIFIER]   = { identifier, NULL,   PREC_NONE },
    [T_STRING]       = { string,     NULL,   PREC_NONE },
    [T_NUMBER]       = { number,     NULL,   PREC_NONE },
    [T_BOOLEAN]      = { NULL,       NULL,   PREC_NONE },
    [T_AND]          = { NULL,       NULL,   PREC_NONE },
    [T_OR]           = { NULL,       NULL,   PREC_NONE },
    [T_FUN]          = { NULL,       NULL,   PREC_NONE },
    [T_IF]           = { NULL,       NULL,   PREC_NONE },
    [T_ELSE]         = { NULL,       NULL,   PREC_NONE },
    [T_FOR]          = { NULL,       NULL,   PREC_NONE },
    [T_WHILE]        = { NULL,       NULL,   PREC_NONE },
    [T_VAR]          = { assignment, NULL,   PREC_NONE },
    [T_TRUE]         = { NULL,       NULL,   PREC_NONE },
    [T_FALSE]        = { NULL,       NULL,   PREC_NONE },
    [T_LPAREN]       = { grouping,   NULL,   PREC_NONE },
    [T_RPAREN]       = { NULL,       NULL,   PREC_NONE },
    [T_LBRACKET]     = { NULL,       NULL,   PREC_NONE },
    [T_RBRACKET]     = { NULL,       NULL,   PREC_NONE },
    [T_LCURLY]       = { NULL,       NULL,   PREC_NONE },
    [T_RCURLY]       = { NULL,       NULL,   PREC_NONE },
    [T_QUOTE]        = { NULL,       NULL,   PREC_NONE },
    [T_DBL_QUOTE]    = { NULL,       NULL,   PREC_NONE },
    [T_SEMICOLON]    = { NULL,       NULL,   PREC_NONE },
    [T_COMMA]        = { NULL,       NULL,   PREC_NONE },
    [T_DOT]          = { NULL,       NULL,   PREC_NONE },
    [T_HASHTAG]      = { NULL,       NULL,   PREC_NONE },
    [T_CARET]        = { NULL,       NULL,   PREC_NONE },
    [T_PIPE]         = { NULL,       NULL,   PREC_NONE },
    [T_AMP]          = { NULL,       NULL,   PREC_NONE },
    [T_EQL]          = { NULL,       NULL,   PREC_NONE },
    [T_DBL_EQL]      = { NULL,       NULL,   PREC_NONE },
    [T_BANG]         = { NULL,       NULL,   PREC_NONE },
    [T_BANG_EQL]     = { NULL,       NULL,   PREC_NONE },
    [T_GREATER]      = { NULL,       NULL,   PREC_NONE },
    [T_LESS]         = { NULL,       NULL,   PREC_NONE },
    [T_GREATER_EQL]  = { NULL,       NULL,   PREC_NONE },
    [T_LESS_EQL]     = { NULL,       NULL,   PREC_NONE },
    [T_PLUS]         = { NULL,       binary, PREC_TERM },
    [T_MINUS]        = { unary,      binary, PREC_TERM },
    [T_ASTERISK]     = { NULL,       binary, PREC_FACTOR },
    [T_SLASH]        = { NULL,       binary, PREC_FACTOR },
    [T_PLUS_PLUS]    = { NULL,       NULL,   PREC_NONE },
    [T_MINUS_MINUS]  = { NULL,       NULL,   PREC_NONE },
    [T_PLUS_EQL]     = { NULL,       NULL,   PREC_NONE },
    [T_MINUS_EQL]    = { NULL,       NULL,   PREC_NONE },
    [T_ASTERISK_EQL] = { NULL,       NULL,   PREC_NONE },
    [T_SLASH_EQL]    = { NULL,       NULL,   PREC_NONE },
};

/* helper functions */
static const Rule *const get_rule(Token *token) {
    return &rules[token->type];
}

static void advance(ParserState *parser) {
    parser->prev = parser->current;
    parser->current++;
}

static int expect(ParserState *parser, TokenType type) {
    if (parser->current == NULL || parser->current->type != type) {
        parser->error = 1;
        return 0;
    }
    return 1;
}

static int at_end(ParserState *parser) {
    return parser->current->type == T_EOF;
}

static void index_of(NameArray *names, char *str, int *ind) {
    for (unsigned int i = 0; i < names->elements; i++) {
        if (strcmp(names->array[i], str) == 0) {
            *ind = i;
            return;
        }
    }

    *ind = -1;
}

static void emit_opcode(BytecodeArray *array, opcode_t op) {
    append_to_bytecode_dynarray(array, op);
}

static void emit_constant(BytecodeArray *array, Value val) {
    append_to_bytecode_dynarray(array, OP_CONSTANT);
    append_to_bytecode_dynarray(array, array->constants->elements); // index of constant
    append_to_value_dynarray(array->constants, val);
}

static void emit_set_name(ParserState *parser, char *str) {
    BytecodeArray *code = parser->bytecode;
    int ind = -1;
    index_of(code->names, str, &ind);
    append_to_bytecode_dynarray(code, OP_SET_GLOBAL);

    if (ind == -1) {
        append_to_bytecode_dynarray(code, code->names->elements); // index of constant
        append_to_name_dynarray(code->names, str);    
    } else {
        append_to_bytecode_dynarray(code, ind);
    }
}

static void emit_get_name(ParserState *parser, char *str) {
    BytecodeArray *code = parser->bytecode;
    int ind = -1;
    index_of(code->names, str, &ind);
    append_to_bytecode_dynarray(code, OP_GET_GLOBAL);

    if (ind == -1) {
        append_to_bytecode_dynarray(code, code->names->elements); // index of constant
        append_to_name_dynarray(code->names, str);    
    } else {
        append_to_bytecode_dynarray(code, ind);
    }
}

/* parsing functions */
static void parse_precedence(ParserState *parser, Precedence prec) {
    #ifdef DEBUG_PARSER
    printf("in parse_precedence\n");
    #endif

    ParsingFunction prefix = get_rule(parser->current)->prefix;

    if (prefix == NULL) {
        printf("expected expression\n");
        return;
    }

    prefix(parser);

    while (!at_end(parser) && prec <= get_rule(parser->current)->precedence) {
        ParsingFunction infix = get_rule(parser->current)->infix;
        infix(parser);
    }
}

static void expression(ParserState *parser) {
    parse_precedence(parser, PREC_ASSIGNMENT);
}

static void binary(ParserState *parser) {
    #ifdef DEBUG_PARSER
    printf("in binary\n");
    #endif

    Token *operator = parser->current;
    advance(parser);
    parse_precedence(parser, get_rule(operator)->precedence + 1);

    switch (operator->type) {
        case T_PLUS: emit_opcode(parser->bytecode, OP_ADD); break;
        case T_MINUS: emit_opcode(parser->bytecode, OP_SUB); break;
        case T_ASTERISK: emit_opcode(parser->bytecode, OP_MULT); break;
        case T_SLASH: emit_opcode(parser->bytecode, OP_DIV); break;
        default:
        break;
    }

    #ifdef DEBUG_PARSER
    printf("out of binary\n");
    #endif
}

static void unary(ParserState *parser) {
    #ifdef DEBUG_PARSER
    printf("in unary\n");
    #endif

    Token *operator = parser->current;
    advance(parser);
    parse_precedence(parser, get_rule(operator)->precedence + 1);

    switch (operator->type) {
        case T_MINUS: emit_opcode(parser->bytecode, OP_SUB); break;
        default:
        break;
    }
    #ifdef DEBUG_PARSER
    printf("out of unary\n");
    #endif
}

static void identifier(ParserState *parser) {
    emit_get_name(parser, parser->current->value);
    advance(parser);
}

static void number(ParserState *s) {
    #ifdef DEBUG_PARSER
    printf("in number\n");
    #endif

    emit_constant(s->bytecode, double_value(atof(s->current->value)));
    advance(s);

    #ifdef DEBUG_PARSER
    printf("out of number\n");
    #endif
}

static void string(ParserState *s) {
    #ifdef DEBUG_PARSER
    printf("in number\n");
    #endif

    emit_constant(s->bytecode, string_value(s->current->value));
    advance(s);

    #ifdef DEBUG_PARSER
    printf("out of number\n");
    #endif
}

static void grouping(ParserState *s) {
    advance(s);
    expression(s);

    if (!expect(s, T_RPAREN)) {
        report_error("SyntaxError", "Expected closing parenthesis ')'");
        return;
    } else {
        advance(s);
    }
}

/* var x = expr */
static void assignment(ParserState *parser) {
    advance(parser);
    /* expect identifier, push name */
    if (!expect(parser, T_IDENTIFIER)) {
        report_error("SyntaxError", "Expected identifier in assignment statement");
        return;
    }

    Token *name = parser->current;

    advance(parser);
    if (!expect(parser, T_EQL)) {
        report_error("SyntaxError", "Expected '=' after variable in assignment statement");
        return;
    }

    advance(parser);
    expression(parser);

    emit_set_name(parser, name->value);
}
