#ifndef _TOKENS_H_
#define _TOKENS_H_

enum token_type {
    T_NONE,

    T_IDENTIFIER,
    T_STRING,
    T_NUMBER,

    T_AND,
    T_OR,
    T_FUN,
    T_IF,
    T_ELSE,
    T_FOR,
    T_WHILE,

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
};

typedef struct {
    char *value;
    enum token_type type;
    unsigned int line;
} token;

typedef struct {
    token *tokens;
    unsigned int capacity;
    unsigned int count;
} token_dynamic_array;

/* token ops */
void print_token(token *t);
token create_token();
void free_token(token *t);

/* array ops */
token_dynamic_array *create_token_dyn_array();
void append_to_array(token_dynamic_array *array, token *t);
void free_array(token_dynamic_array *array);
void print_tokens(token_dynamic_array *array);

#endif /* _TOKENS_H_ */