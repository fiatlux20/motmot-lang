/** \file lexer.h
 * Contains functions related to turning source code into tokens.
 */
#ifndef _JITCALC_LEXER_H_
#define _JITCALC_LEXER_H_

typedef struct function_table FunctionTable;

typedef enum {
    T_ERROR,
    T_END,
    T_NUMBER,
    T_VAR,
    T_EQUALS,
    T_DBLEQL,
    T_LPAREN,
    T_RPAREN,
    T_FUNC,
    T_FUNCALL,
    T_PLUS,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_MOD,
    T_SQRT
} TokenType;

typedef struct token {
    TokenType type;
    union {
        float value;
        char *string;
    };
} Token;

typedef struct token_array {
    Token *tokens;
    char *var_name;
    unsigned int elements;
    unsigned int capacity;
    unsigned int num_constants;
    unsigned char in_function : 1;
} TokenArray;

/**
 * Takes a string of source code and a pointer to an existing functions table
 * and returns an array of tokens of the form { TokenType, Value } where Value
 * is a union containing either a float or a string pointer. The token array
 * either ends with the token type END if there were no errors, or ERROR if
 * there were, typically the previous token being the token right before the
 * error was caused.
 *
 * This function also takes a pointer to a function table to perform some
 * checking for redefining already existing names or aliasing function names
 * with variable names.
 *
 * @param code A source code string to be tokenized.
 * @param functions A function table which contains all previously defined
 *                  functions
 */
TokenArray *tokenize(char *code, FunctionTable *functions);

/**
 * Returns if the token array contains an error
 *
 * @param array The token array to check
 */
int tokenizer_had_error(TokenArray *array);

/**
 * Frees all heap allocated memory used by the TokenArray struct and sets the
 * pointer to NULL.
 *
 * see <a href="https://en.wikipedia.org/wiki/Main_Page">this</a>
 *
 * @param array The token array to free. Set to NULL after the function returns
 */
void free_token_array(TokenArray *array);

#ifdef DEBUG
void print_tokens(TokenArray *tokens);
#endif

#endif /* _JITCALC_LEXER_H_ */
