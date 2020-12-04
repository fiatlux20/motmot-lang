#ifndef _TOKENS_H_
#define _TOKENS_H_

#include "common.h"

typedef struct rule Rule;
typedef struct token Token;
typedef struct token_dynamic_array TokenArray;

typedef void (*ParsingFunction)();

typedef enum {
    T_NONE,
    T_EOF,
    T_ERROR,
    T_NIL,

    T_IDENTIFIER,
    T_STRING,
    T_NUMBER,
    T_BOOLEAN,

    T_AND,
    T_OR,
    T_FUN,
    T_IF,
    T_ELSE,
    T_FOR,
    T_WHILE,
    T_VAR,
    T_TRUE,
    T_FALSE,

    T_LPAREN,
    T_RPAREN,
    T_LBRACKET,
    T_RBRACKET,
    T_LCURLY,
    T_RCURLY,

    T_QUOTE,
    T_DBL_QUOTE,
    T_SEMICOLON,

    T_COMMA,
    T_DOT,

    T_HASHTAG,
    T_CARET,
    T_PIPE,
    T_AMP,
    T_EQL,
    T_DBL_EQL,
    T_BANG,
    T_BANG_EQL,
    T_GREATER,
    T_LESS,
    T_GREATER_EQL,
    T_LESS_EQL,

    T_PLUS,
    T_MINUS,
    T_ASTERISK,
    T_SLASH,
    T_PLUS_PLUS,
    T_MINUS_MINUS,
    T_PLUS_EQL,
    T_MINUS_EQL,
    T_ASTERISK_EQL,
    T_SLASH_EQL
} TokenType;

typedef enum {
    PREC_NONE,
    PREC_TERM,
    PREC_FACTOR
} Precedence;

struct rule {
    ParsingFunction prefix;
    ParsingFunction infix;
    Precedence precedence;
};

struct token {
    char *value;
    TokenType type;
    unsigned int line;
};

struct token_dynamic_array {
    Token *tokens;
    unsigned int capacity;
    unsigned int count;
};

/* token ops */
void print_token(Token *t);
Token create_token();
void free_token(Token *t);

/* array ops */
TokenArray *create_token_dyn_array();
void append_to_array(TokenArray *array, Token *t);
void free_array(TokenArray *array);
void print_tokens(TokenArray *array);
void print_tokens2(TokenArray *array);

Token *next_token(TokenArray *array, dynarray_iterator *iter);
Token *peek_next_token(TokenArray *array, dynarray_iterator *iter);
Token *current_token(TokenArray *array, dynarray_iterator *iter);

#endif /* _TOKENS_H_ */
