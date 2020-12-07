#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static void advance(ParserState*);
static void parse_precedence(ParserState*, Precedence);
static void expression(ParserState*);
static void binary(ParserState*);
static void grouping(ParserState*);
static void unary(ParserState*);
static void number(ParserState*);
static void identifier(ParserState*);

const static Rule rules[] = {
    [T_NONE]         = { NULL,       NULL,   PREC_NONE },
    [T_EOF]          = { NULL,       NULL,   PREC_NONE },
    [T_ERROR]        = { NULL,       NULL,   PREC_NONE },
    [T_NIL]          = { NULL,       NULL,   PREC_NONE },
    [T_IDENTIFIER]   = { identifier, NULL,   PREC_NONE },
    [T_STRING]       = { NULL,       NULL,   PREC_NONE },
    [T_NUMBER]       = { number,     NULL,   PREC_NONE },
    [T_BOOLEAN]      = { NULL,       NULL,   PREC_NONE },
    [T_AND]          = { NULL,       NULL,   PREC_NONE },
    [T_OR]           = { NULL,       NULL,   PREC_NONE },
    [T_FUN]          = { NULL,       NULL,   PREC_NONE },
    [T_IF]           = { NULL,       NULL,   PREC_NONE },
    [T_ELSE]         = { NULL,       NULL,   PREC_NONE },
    [T_FOR]          = { NULL,       NULL,   PREC_NONE },
    [T_WHILE]        = { NULL,       NULL,   PREC_NONE },
    [T_VAR]          = { NULL,       NULL,   PREC_NONE },
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


/* public functions */
BytecodeArray *parse(VirtualMachine *vm, TokenArray *tokens) {
    dynarray_iterator iter = { tokens->count, 0 };
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
static const Rule *const get_rule(Token *token) {
    return &rules[token->type];
}

static void advance(ParserState *parser) {
    parser->prev = parser->current;
    parser->current++;
}

void emit_opcode(BytecodeArray *array, opcode_t op) {
    append_to_bytecode_dynarray(array, op);
}

void emit_constant(BytecodeArray *array, Value val) {
    append_to_bytecode_dynarray(array, OP_CONSTANT);
    append_to_bytecode_dynarray(array, array->constants->elements); // index of constant
    append_to_value_dynarray(array->constants, val);
}

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

    while (prec <= get_rule(parser->current)->precedence) {
        ParsingFunction infix = get_rule(parser->current)->infix;
        infix(parser);
    }
}

static void expression(ParserState *parser) {
    parse_precedence(parser, PREC_ASSIGNMENT);
}

static void identifier(ParserState *parser) {

}

static void grouping(ParserState *parser) {
    
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
