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


static unsigned int operator(Token *t) {
    if (t == NULL) {
        return 0;
    }

    unsigned int type = t->type;

    return type == T_PLUS     || type == T_MINUS
        || type == T_ASTERISK || type == T_SLASH
        || type == T_GREATER  || type == T_LESS
        || type == T_BANG;
}

static unsigned int is_value(Token *t) {
    if (t == NULL) {
        return 0;
    }

    unsigned int type = t->type;

    return type == T_IDENTIFIER || type == T_NUMBER || type == T_STRING;
}

static unsigned int accept(Token *t, unsigned int token_type) {
    if (t == NULL) {
        return 0;
    }
    return t->type == token_type;
}

static unsigned int expect(ParserState *s, unsigned int token_type) {
    Token *t = current_token(s->tokens, s->iter);
    if (t == NULL || t->type != token_type) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
}

static unsigned int expect_operator(ParserState *s) {
    Token *t = current_token(s->tokens, s->iter);
    if (t == NULL || !operator(t)) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
}

static unsigned int expect_value(ParserState *s) {
    Token *t = current_token(s->tokens, s->iter);
    if (t == NULL || !is_value(t)) {
        s->error = 1;
        return 0;
    }

    next_token(s->tokens, s->iter);
    return 1;
}

/* bytecode functions */
void index_of(NameArray *names, char *str, int *ind) {
    for (unsigned int i = 0; i < names->elements; i++) {
        if (strcmp(names->array[i], str) == 0) {
            *ind = i;
            return;
        }
    }

    *ind = -1;
}
void emit_opcode(BytecodeArray *array, opcode_t opcode) {
    append_to_bytecode_dynarray(array, opcode);
}

void emit_constant(BytecodeArray *array, Value val) {
    append_to_bytecode_dynarray(array, OP_CONSTANT);
    append_to_bytecode_dynarray(array, array->constants.elements); // index of constant
    append_to_value_dynarray(&(array->constants), val);
}

void emit_set_name(BytecodeArray *array, char *str) {
    int ind = -1;
    index_of(array->names, str, &ind);
    append_to_bytecode_dynarray(array, OP_SET_GLOBAL);

    if (ind == -1) {
        append_to_bytecode_dynarray(array, array->names->elements); // index of constant
        append_to_name_dynarray(array->names, str);    
    } else {
        append_to_bytecode_dynarray(array, ind);
    }

}

void emit_update_name(BytecodeArray *array, char *str) {
    int ind = -1;
    index_of(array->names, str, &ind);
    append_to_bytecode_dynarray(array, OP_UPDATE_GLOBAL);

    if (ind == -1) {
        append_to_bytecode_dynarray(array, array->names->elements); // index of constant
        append_to_name_dynarray(array->names, str);    
    } else {
        append_to_bytecode_dynarray(array, ind);
    }

}

void emit_get_name(BytecodeArray *array, char *str) {
    int ind = -1;
    index_of(array->names, str, &ind);
    append_to_bytecode_dynarray(array, OP_GET_GLOBAL);

    if (ind == -1) {
        append_to_bytecode_dynarray(array, array->names->elements); // index of constant
        append_to_name_dynarray(array->names, str);    
    } else {
        append_to_bytecode_dynarray(array, ind);
    }
}

/* matching groups */
static void binary(ParserState *s);
static void unary(ParserState *s);
static void grouping(ParserState *s);
static void block(ParserState *s);
static void expression(ParserState *s);
static void statement(ParserState *s);
static void if_statement(ParserState *s);

static void binary(ParserState *s) {
    #ifdef DEBUG_PARSER
    printf("in binary\n");
    #endif

    Token *operator = current_token(s->tokens, s->iter);

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

static void unary(ParserState *s) {
    #ifdef DEBUG_PARSER
    printf("in unary\n");
    #endif

    expect_operator(s);
    expression(s);

    #ifdef DEBUG_PARSER
    printf("out of unary\n");
    #endif
}

static void grouping(ParserState *s) {
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

static void block(ParserState *s) {
    expect(s, T_LCURLY);
    statement(s);
    expect(s, T_RCURLY);
}

static void expression(ParserState *s) {
    #ifdef DEBUG_PARSER
    printf("in expression\n");
    #endif

    Token *t = current_token(s->tokens, s->iter);
    if (t == NULL) {
        return;
    }

    if (operator(t)) {
        unary(s);
        return;
    }

    Token *next = peek_next_token(s->tokens, s->iter);
    if (next == NULL) {
        return;
    }

    switch (t->type) {
    case T_NUMBER:
        emit_constant(s->bytecode, double_value(atof(t->value)));
        next_token(s->tokens, s->iter);
        break;
    case T_IDENTIFIER:
        emit_get_name(s->bytecode, t->value);
        next_token(s->tokens, s->iter);
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

static void assignment(ParserState *s) {
    expect(s, T_VAR);
    Token *name = current_token(s->tokens, s->iter);
    expect(s, T_IDENTIFIER);
    expect(s, T_EQL);
    expression(s);

    emit_set_name(s->bytecode, name->value);
}

static void statement(ParserState *s) {
    Token *t = current_token(s->tokens, s->iter);
    Token *temp;

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
    case T_IDENTIFIER:
        if (peek_next_token(s->tokens, s->iter)->type == T_EQL) {
            temp = current_token(s->tokens, s->iter);
            next_token(s->tokens, s->iter);
            next_token(s->tokens, s->iter);
            expression(s);
            emit_update_name(s->bytecode, temp->value);
            next_token(s->tokens, s->iter);
        }
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

static void if_statement(ParserState *s) {
    expect(s, T_IF);

    expect(s, T_LPAREN);
    expression(s);
    expect(s, T_RPAREN);

    statement(s);
}

/* public functions */
BytecodeArray parse(VirtualMachine *vm, TokenArray *tokens) {
    dynarray_iterator iter = { tokens->count, 0 };
    BytecodeArray bytecode = create_bytecode_dynarray();
    bytecode.names = &vm->names;

    ParserState s;
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
