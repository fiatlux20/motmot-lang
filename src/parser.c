#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/* program -> statement
 *
 * statement -> '{' statement_list '}'
 *            | IF '(' expr ')' statement
 *            | VAR identifier EQL expr ;
 *
 * statement_list -> statement
 *                 | statement_list ';' statement
 *
 * expr -> identifier
 *       | literal
 *       | unary
 *       | binary
 *       | grouping
 * 
 * binary -> expr operator expr
 * 
 * grouping -> LPAREN expr RPAREN
 * 
 *
 * literal -> number
 *          | string
 *          | boolean
 *
 * operator -> plus
 *           | minus
 */


static unsigned int operator(token *t) {
    if (t == NULL) {
        return 0;
    }

    unsigned int type = t->type;

    return type == T_PLUS     || type == T_MINUS
        || type == T_ASTERISK || type == T_SLASH
        || type == T_GREATER  || type == T_LESS
        || type == T_BANG;
}

static unsigned int is_value(token *t) {
    if (t == NULL) {
        return 0;
    }

    unsigned int type = t->type;

    return type == T_IDENTIFIER || type == T_NUMBER || type == T_STRING;
}

static void report_error(const char *error_type, const char *message) {
    fprintf(stderr, "\033[0;31m%s\033[0m: ", error_type);
    fputs(message, stderr);
    fputs("\n", stderr);
}

static unsigned int accept(token *t, unsigned int token_type) {
    if (t == NULL) {
        return 0;
    }
    return t->type == token_type;
}

static unsigned int expect(parser_state *s, unsigned int token_type) {
    token *t = current_token(s->tokens, s->iter);
    if (t == NULL || t->type != token_type) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
}

static unsigned int expect_operator(parser_state *s) {
    token *t = current_token(s->tokens, s->iter);
    if (t == NULL || !operator(t)) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
}

static unsigned int expect_value(parser_state *s) {
    token *t = current_token(s->tokens, s->iter);
    if (t == NULL || !is_value(t)) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
}

/* bytecode functions */
void emit_opcode(bytecode_array *array, opcode_t opcode) {
    append_to_bytecode_dynarray(array, opcode);
}

void emit_constant(bytecode_array *array, Value val) {
    append_to_bytecode_dynarray(array, OP_CONSTANT);
    append_to_bytecode_dynarray(array, array->constants.elements); // index of constant
    append_to_value_dynarray(&(array->constants), val);
}

/* matching groups */
static void binary(parser_state *s);
static void unary(parser_state *s);
static void grouping(parser_state *s);
static void block(parser_state *s);
static void expression(parser_state *s);
static void statement(parser_state *s);
static void if_statement(parser_state *s);

static void binary(parser_state *s) {
    #ifdef DEBUG_PARSER
    printf("in binary\n");
    #endif

    token *operator = current_token(s->tokens, s->iter);

    expect_operator(s);
    expression(s);

    switch (operator->type) {
        case T_PLUS: emit_opcode(s->bytecode, OP_ADD); break;
        case T_MINUS: emit_opcode(s->bytecode, OP_SUB); break;
        case T_ASTERISK: emit_opcode(s->bytecode, OP_MULT); break;
        case T_SLASH: emit_opcode(s->bytecode, OP_DIV); break;
        default:
            break;
    }

    #ifdef DEBUG_PARSER
    printf("out of binary\n");
    #endif
}

static void unary(parser_state *s) {
    #ifdef DEBUG_PARSER
    printf("in unary\n");
    #endif

    expect_operator(s);
    expression(s);

    #ifdef DEBUG_PARSER
    printf("out of unary\n");
    #endif
}

static void grouping(parser_state *s) {
    #ifdef DEBUG_PARSER
    printf("in grouping\n");
    #endif

    expect(s, T_LPAREN);
    expression(s);
    expect(s, T_RPAREN);

    #ifdef DEBUG_PARSER
    printf("out of grouping\n");
    #endif

    if (s->error != 0) {
        report_error("SyntaxError", "Missing parenthesis.");
        s->error = 0;
    }
}

static void block(parser_state *s) {
    expect(s, T_LCURLY);
    statement(s);
    expect(s, T_RCURLY);
}

static void expression(parser_state *s) {
    #ifdef DEBUG_PARSER
    printf("in expression\n");
    #endif

    token *t = current_token(s->tokens, s->iter);
    if (t == NULL) {
        return;
    }

    if (operator(t)) {
        unary(s);
        return;
    }

    token *next = peek_next_token(s->tokens, s->iter);
    if (next == NULL) {
        return;
    }

    switch (t->type) {
    case T_NUMBER:
        emit_constant(s->bytecode, double_value(atof(t->value)));
        next_token(s->tokens, s->iter);
        break;
    case T_IDENTIFIER:
        break;
    case T_STRING:
        emit_constant(s->bytecode, string_value(t->value));
 // case T_BOOLEAN:
        next_token(s->tokens, s->iter);
        // if (operator(next)) {
        //     binary(s);
        // }
        break;
    case T_LPAREN:
        grouping(s);
        break;
    default:
        next_token(s->tokens, s->iter);
        break;
    }

    t = current_token(s->tokens, s->iter);
    if (operator(t)) {
        binary(s);
    }

    if (s->error != 0) {
        if (t != NULL) {
            report_error("SyntaxError", "invalid token while parsing expression");
            fprintf(stderr, "%d\n", t->type);
        }
        report_error("SyntaxError", "Null token encountered while parsing expression.");
        s->error = 0;
    }
}

static void assignment(parser_state *s) {
    expect(s, T_VAR);
    expect(s, T_IDENTIFIER);
    expect(s, T_EQL);
    expression(s);
}

static void statement(parser_state *s) {
    token *t = current_token(s->tokens, s->iter);

    if (t == NULL) {
        return;
    }

    switch (t->type) {
    case T_IF:
        if_statement(s);
        return;
    case T_VAR:
        assignment(s);
        break;
    case T_LCURLY:
        block(s);
        return;
    default:
        expression(s);
        if (s->error != 0) {
            report_error("SyntaxError", "invalid token encountered while parsing statement.");
            s->error = 0;
        }
        break;
    }

    expect(s, T_SEMICOLON);
    if (s->error != 0) {
        report_error("SyntaxError", "Missing semicolon in statement.");
        s->error = 0;
    }

    if (s->error != 0) {
        report_error("SyntaxError", "invalid token while parsing statement");
        s->error = 0;
    }
}

static void if_statement(parser_state *s) {
    expect(s, T_IF);

    expect(s, T_LPAREN);
    expression(s);
    expect(s, T_RPAREN);

    statement(s);
}

/* public functions */
bytecode_array parse(token_dynamic_array *tokens) {
    dynarray_iterator iter = { tokens->count, 0 };
    bytecode_array bytecode = create_bytecode_dynarray();
    parser_state s;
    s.tokens = tokens;
    s.bytecode = &bytecode;
    s.iter = &iter;
    s.error = 0;

    // expression(&s);
    statement(&s);

    #ifdef DEBUG_PARSER
    printf("index at %d out of %d\n", iter.index + 1, tokens->count);
    #endif

    return bytecode;
}
