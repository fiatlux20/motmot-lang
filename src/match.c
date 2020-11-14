#include <string.h>

#include "match.h"


unsigned int match_keyword(char *word) {
    if (strcmp(word, "and") == 0) {
        return AND;
    } if (strcmp(word, "or") == 0) {
        return OR;
    } if (strcmp(word, "fun") == 0) {
        return FUN;
    } if (strcmp(word, "if") == 0) {
        return IF;
    } if (strcmp(word, "else") == 0) {
        return ELSE;
    } if (strcmp(word, "for") == 0) {
        return FOR;
    } if (strcmp(word, "while") == 0) {
        return WHILE;
    }

    return 0;
}

unsigned int match_symbol(char *source, unsigned int source_size, unsigned int *ind) {
    switch(source[*ind]) {
    case '(': return LPAREN;
    case ')': return RPAREN;
    case '[': return LBRACKET;
    case ']': return RBRACKET;
    case '{': return LCURLY;
    case '}': return RCURLY;
    case ';': return SEMICOLON;
    case 39:  return QUOTE;    /* ascii for ' */
    case '"': return DBL_QUOTE;
    case ',': return COMMA;
    case '.': return DOT;

    case '=':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return DBL_EQL;
        } else {
            return EQL;
        } break;
    case '!':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return BANG_EQL;
        } else {
            return BANG;
        } break;
    case '>':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return GREATER_EQL;
        } else {
            return GREATER;
        } break;
    case '<':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return LESS_EQL;
        } else {
            return LESS;
        } break;

    case '+':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return PLUS_EQL;
        } else if ((*ind + 1) < source_size && source[*ind + 1] == '+') {
            (*ind)++; return PLUS_PLUS;
        } else {
            return PLUS;
        } break;
    case '-':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return MINUS_EQL;
        } else if ((*ind + 1) < source_size && source[*ind + 1] == '-') {
            (*ind)++; return MINUS_MINUS;
        } else {
            return MINUS;
        } break;

    case '*':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return ASTERISK_EQL;
        } else {
            return ASTERISK;
        } break;
    case '/':
        if ((*ind + 1) < source_size && source[*ind + 1] == '=') {
            (*ind)++; return SLASH_EQL;
        } else {
            return SLASH;
        } break;
    }

    return 0;
}