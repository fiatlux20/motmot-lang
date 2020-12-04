#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"


/* public token ops */
void print_token(Token *token) {
    if (token == NULL) {
        printf("NULL TOKEN\n");
        return;
    }

    if (token->value != NULL) {
        switch (token->type) {
        case T_NUMBER: { printf("(NUMBER, '%s')\n", token->value); break; } 
        case T_IDENTIFIER: { printf("(IDENTIFIER, '%s')\n", token->value); break; }
        case T_STRING: { printf("(STRING, '%s')\n", token->value); break; }
        case T_BOOLEAN: { printf("(BOOLEAN, '%s')\n", token->value); break; }
        default: break;
        }
    } else {
        const char *symbol;

        switch(token->type) {
        case T_NIL: symbol = "NIL"; break;
        case T_AND: symbol = "AND"; break;
        case T_OR: symbol = "OR"; break;
        case T_FUN: symbol = "FUN"; break;
        case T_IF: symbol = "IF"; break;
        case T_ELSE: symbol = "ELSE"; break;
        case T_FOR: symbol = "FOR"; break;
        case T_WHILE: symbol = "WHILE"; break;
        case T_VAR: symbol = "VAR"; break;

        case T_LPAREN: symbol = "LPAREN"; break;
        case T_RPAREN: symbol = "RPAREN"; break;
        case T_LBRACKET: symbol = "LBRACKET"; break;
        case T_RBRACKET: symbol = "RBRACKET"; break;
        case T_LCURLY: symbol = "LCURLY"; break;
        case T_RCURLY: symbol = "RCURLY"; break;

        case T_QUOTE: symbol = "QUOTE"; break;
        case T_DBL_QUOTE: symbol = "DBL_QUOTE"; break;
        case T_SEMICOLON: symbol = "SEMICOLON"; break;
        case T_COMMA: symbol = "COMMA"; break;
        case T_DOT: symbol = "DOT"; break;

        case T_HASHTAG: symbol = "HASHTAG"; break;
        case T_CARET: symbol = "CARET"; break;
        case T_PIPE: symbol = "PIPE"; break;
        case T_AMP: symbol = "AMP"; break;
        case T_EQL: symbol = "EQL"; break;
        case T_DBL_EQL: symbol = "DBL_EQL"; break;
        case T_BANG: symbol = "BANG"; break;
        case T_BANG_EQL: symbol = "BANG_EQL"; break;
        case T_GREATER: symbol = "GREATER"; break;
        case T_LESS: symbol = "LESS"; break;
        case T_GREATER_EQL: symbol = "GREATER_EQL"; break;
        case T_LESS_EQL: symbol = "LESS_EQL"; break;

        case T_PLUS: symbol = "PLUS"; break;
        case T_MINUS: symbol = "MINUS"; break;
        case T_ASTERISK: symbol = "ASTERISK"; break;
        case T_SLASH: symbol = "SLASH"; break;
        case T_PLUS_PLUS: symbol = "PLUS_PLUS"; break;
        case T_MINUS_MINUS: symbol = "MINUS_MINUS"; break;
        case T_PLUS_EQL: symbol = "PLUS_EQL"; break;
        case T_MINUS_EQL: symbol = "MINUS_EQL"; break;
        case T_ASTERISK_EQL: symbol = "ASTERISK_EQL"; break;
        case T_SLASH_EQL: symbol = "SLASH_EQL"; break;
        default:
            symbol = "UNDEF";
        }

        printf("(%s, %d)\n", symbol, token->type);
    }
}

Token create_token() {
    Token t;
    t.value = NULL;
    t.type = T_NONE;
    t.line = 0;

    return t;
}

void free_token(Token *t) {
    free(t->value);
    t->value = NULL;
    return;
}

/* dynamic array utils */
static void grow_array(TokenArray *array, unsigned int new_size) {
    array->tokens = realloc(array->tokens, (sizeof *array->tokens) * new_size);
    if (array->tokens == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }
}


/* public dynarray functions */
TokenArray *create_token_dyn_array() {
    TokenArray *array = malloc(sizeof *array);
    array->capacity = DYNARRAY_INITIAL_SIZE;
    array->count = 0;
    array->tokens = malloc((sizeof *array->tokens) * array->capacity);
    return array;
}


void append_to_array(TokenArray *array, Token *t) {
    if (array->count == array->capacity) {
        array->capacity *= DYNARRAY_GROW_BY_FACTOR;
        grow_array(array, array->capacity);

        if (array == NULL) {
            printf("got NULL while performing array realloc\n");
            return;
        }
    }

    array->tokens[array->count] = *t;
    array->count++;
}

void free_array(TokenArray *array) {
    for (int i = 0; i < array->count; i++) {
        free(array->tokens[i].value);
        array->tokens[i].value = NULL;
    }

    free(array->tokens);
    array->tokens = NULL;

    free(array);
    array = NULL;
}

void print_tokens(TokenArray *array) {
    for (unsigned int i = 0; i < array->count; i++) {
        print_token(&(array->tokens[i]));
    }
}

Token *next_token(TokenArray *array, dynarray_iterator *iter) {
    if (iter->index >= array->count) {
        return NULL;
    }

    #ifdef DEBUG_TOKENS
    if (peek_next_token(array, iter) != NULL) {
        printf("advancing to token: ");
        print_token(&array->tokens[iter->index + 1]);
    }
    #endif

    if (iter->index + 1 < array->count) {
        iter->index++;
    }

    return &(array->tokens[iter->index]);
}

Token *peek_next_token(TokenArray *array, dynarray_iterator *iter) {
    if (iter->index + 1 >= array->count) {
        return NULL;
    }

    #ifdef DEBUG_TOKENS
    printf("peeking next token: ");
    print_token(&array->tokens[iter->index + 1]);
    #endif

    return &(array->tokens[iter->index + 1]);
}

Token *current_token(TokenArray *array, dynarray_iterator *iter) {
    if (iter->index >= array->count) {
        return NULL;
    }

    #ifdef DEBUG_TOKENS
    printf("current token: ");
    print_token(&array->tokens[iter->index]);
    #endif

    return &(array->tokens[iter->index]);
}

#define foreach(var, array) \
    dynarray_iterator _iter = { array->count, 0 }; \
    while ((var = next_token(array, &_iter)) != NULL)


// #define foreach(var, array, iter) while ((var = next_token(array, iter)) != NULL)
void print_tokens2(TokenArray *array) {
    Token *t;
    // dynarray_iterator iter = { array->count, 0 };

    foreach(t, array) {
        print_token(t);
    }
}
#undef foreach
