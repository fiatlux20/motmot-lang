#include <string.h>

#include "match.h"


unsigned int match_keyword(char *word) {
    if (word == NULL) {
        return 0;
    }

    switch (word[0]) {
    case 'a': if (strcmp(word, "and") == 0)   { return T_AND; }
    case 'e': if (strcmp(word, "else") == 0)  { return T_ELSE; }
    case 'f': if (strcmp(word, "fun") == 0)   { return T_FUN; }
         else if (strcmp(word, "for") == 0)   { return T_FOR; }
    case 'i': if (strcmp(word, "if") == 0)    { return T_IF; }
    case 'o': if (strcmp(word, "or") == 0)    { return T_OR; }
    case 'w': if (strcmp(word, "while") == 0) { return T_WHILE; }
    default:                                  { return 0; }
    }
}

unsigned int match_symbol(char *source, unsigned int source_size, unsigned int *ind) {
    switch(source[*ind]) {
    case '(': return T_LPAREN;
    case ')': return T_RPAREN;
    case '[': return T_LBRACKET;
    case ']': return T_RBRACKET;
    case '{': return T_LCURLY;
    case '}': return T_RCURLY;
    case ';': return T_SEMICOLON;
    case 39:  return T_QUOTE;    /* ascii for ' */
    case '"': return T_DBL_QUOTE;
    case ',': return T_COMMA;
    case '.': return T_DOT;

    case '=':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_DBL_EQL;
        } else {
            return T_EQL;
        } break;
    case '!':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_BANG_EQL;
        } else {
            return T_BANG;
        } break;
    case '>':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_GREATER_EQL;
        } else {
            return T_GREATER;
        } break;
    case '<':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_LESS_EQL;
        } else {
            return T_LESS;
        } break;

    case '+':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_PLUS_EQL;
        } else if ((*ind + 1) < source_size && source[*ind + 1] == '+') {
            (*ind)++; return T_PLUS_PLUS;
        } else {
            return T_PLUS;
        } break;
    case '-':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_MINUS_EQL;
        } else if ((*ind + 1) < source_size && source[*ind + 1] == '-') {
            (*ind)++; return T_MINUS_MINUS;
        } else {
            return T_MINUS;
        } break;

    case '*':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_ASTERISK_EQL;
        } else {
            return T_ASTERISK;
        } break;
    case '/':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return T_SLASH_EQL;
        } else {
            return T_SLASH;
        } break;
    }

    return 0;
}