/** @file tokens.h */
#ifndef _TOKENS_H_
#define _TOKENS_H_

#include "common.h"

typedef struct token Token;
typedef struct token_dynamic_array TokenArray;

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

/**
 * Prints out a token in the form (TYPE) or (TYPE, VALUE)
 *
 * @param t Pointer to a token to print.
 */
void print_token(Token *t);

/**
 * Returns an empty Token struct.
 *
 * @return A Token struct.
 */
Token create_token();

/**
 * Frees a token if its value is not NULL.
 *
 * @param t Pointer to a token to free.
 */
void free_token(Token *t);

/* array ops */
/**
 * Allocates a TokenArray on the heap and initializes its values to defaults.
 * Must be freed with free_array().
 *
 * @return A pointer to a heap-allocated TokenArray.
 */
TokenArray *create_token_dyn_array();

/**
 * Resizes the array if necessary and appends a Token.
 *
 * @param array A token array to add the token to.
 * @param t A token to add to the array.
 */
void append_to_array(TokenArray *array, Token *t);

/**
 * Frees a heap-allocated TokenArray and sets the pointer to NULL.
 *
 * @param array Pointer to the array to free.
 */
void free_array(TokenArray *array);
void print_tokens(TokenArray *array);
void print_tokens2(TokenArray *array);

/**
 * Advances a dynamic array iterator and returns a pointer to the token it is
 * now on.
 *
 * @param array The array the iterator is iterating over.
 * @param iter A dynamic array iterator.
 * @return The next token in the array.
 */
Token *next_token(TokenArray *array, dynarray_iterator *iter);

/**
 * Returns a pointer to the next token from the iterator but does not advance it.
 *
 * @param array The array the iterator is iterating over.
 * @param iter A dynamic array iterator.
 * @return The next token in the array.
 */
Token *peek_next_token(TokenArray *array, dynarray_iterator *iter);

/**
 * Returns a pointer to the token the iterator is currently at.
 *
 * @param array The array the iterator is iterating over.
 * @param iter A dynamic array iterator.
 * @return The next token in the array.
 */
Token *current_token(TokenArray *array, dynarray_iterator *iter);

#endif /* _TOKENS_H_ */
