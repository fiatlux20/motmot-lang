#ifndef _MATCH_H_
#define _MATCH_H_

enum token_type {
    NONE,

    IDENTIFIER,
    NUMBER,

    AND,
    OR,
    FUN,
    IF,
    ELSE,
    FOR,
    WHILE,

    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LCURLY,
    RCURLY,

    QUOTE,
    DBL_QUOTE,
    SEMICOLON,

    COMMA,
    DOT,

    EQL,
    DBL_EQL,
    BANG,
    BANG_EQL,
    GREATER,
    LESS,
    GREATER_EQL,
    LESS_EQL,

    PLUS,
    MINUS,
    ASTERISK,
    SLASH,
    PLUS_PLUS,
    MINUS_MINUS,
    PLUS_EQL,
    MINUS_EQL,
    ASTERISK_EQL,
    SLASH_EQL
};

unsigned int match_keyword(char *word);
unsigned int match_symbol(char *source, unsigned int source_size, unsigned int *ind);

#endif /* _MATCH_H_ */