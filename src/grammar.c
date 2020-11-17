#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"

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

typedef struct {
    token_dynamic_array *tokens;
    token_dynarray_iterator *iter;
    unsigned int error;
} parser_state;

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

static unsigned int value(token *t) {
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
    if (t == NULL || !value(t)) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
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
    printf("in binary\n");
    // expression(s);
    expect_operator(s);
    expression(s);
    printf("out of binary\n");
}

static void unary(parser_state *s) {
    printf("in unary\n");
    expect_operator(s);
    expression(s);
    printf("out of unary\n");
}

static void grouping(parser_state *s) {
    printf("in grouping\n");
    expect(s, T_LPAREN);
    expression(s);
    expect(s, T_RPAREN);
    printf("out of grouping\n");

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
    printf("in expression\n");
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
    case T_IDENTIFIER:
    case T_STRING:
    case T_NUMBER:
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

static void statement(parser_state *s) {
    token *t = current_token(s->tokens, s->iter);

    if (t == NULL) {
        return;
    }

    switch (t->type) {
    case T_IF:
        if_statement(s);
        return;
    case T_LCURLY:
        block(s);
        return;
    default:
        expression(s);
        if (s->error != 0) {
            report_error("SyntaxError", "invalid token encountered while parsing statement.");
            s->error = 0;
        }
        expect(s, T_SEMICOLON);
        if (s->error != 0) {
            report_error("SyntaxError", "Missing semicolon in statement.");
            s->error = 0;
        }
        break;
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
void parse(token_dynamic_array *tokens) {
    token_dynarray_iterator iter = { tokens->count, 0 };
    parser_state s;
    s.tokens = tokens;
    s.iter = &iter;
    s.error = 0;

    // expression(&s);
    statement(&s);

    printf("index at %d out of %d\n", iter.index + 1, tokens->count);

    if (s.error != 0) {
        fprintf(stderr, "got errors\n");
    } else {
        fprintf(stdout, "parsed expression with no errors\n");
    }

    return;
}