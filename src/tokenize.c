#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tokenize.h"


/* util */
static int is_alpha(char c) {
    return ((c >= 97 && c <= 122) || (c >= 65 && c <= 90));
}

static int is_num(char c) {
    return (c >= 48 && c <= 57);
}

static int is_alpha_num(char c) {
    return ((c >= 97 && c <= 122) || (c >= 48 && c <= 57) || (c >= 65 && c <= 90));
}

static int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n');
}

static int is_quote(char c) {
    return c == 39 || c == 34;
}

static int is_symbol(char c) {
    if (is_quote(c)) {
        return 0;
    }
    return (c >= 33 && c <= 47) || (c >=58 && c <= 64) || (c >= 91 && c <= 96) || (c >=123 && c <=126);
}

static unsigned int compare_upto(unsigned int upto, const char *string, const char *match) {
    for (int i = 0; i < upto; i++) {
        if (string[i] == '\0' || string[i] != match[i]) {
            return 0;
        }
    }

    return strcmp(string, match) == 0;
}


/* token matching functions */
static unsigned int match_keyword(char *word) {
    if (word == NULL) {
        return 0;
    }

    switch (word[0]) {
    case 'a': if (strcmp(word, "and") == 0)     { return T_AND; }
    case 'e': if (strcmp(word, "else") == 0)    { return T_ELSE; }
    case 'f': if (compare_upto(2, word, "fun"))   { return T_FUN; }
         else if (compare_upto(2, word, "for"))   { return T_FOR; }
         else if (compare_upto(2, word, "false")) { return T_FALSE; }
    case 'i': if (strcmp(word, "if") == 0)      { return T_IF; }
    case 'n': if (strcmp(word, "nil") == 0)     { return T_NIL; }
    case 'o': if (strcmp(word, "or") == 0)      { return T_OR; }
    case 't': if (strcmp(word, "true") == 0)    { return T_TRUE; }
    case 'v': if (strcmp(word, "var") == 0)     { return T_VAR; }
    case 'w': if (strcmp(word, "while") == 0)   { return T_WHILE; }
    default:                                    { return 0; }
    }
}

static Token match_identifier(char *source, char *buf, unsigned int source_size, unsigned int *pos) {
    unsigned int buf_pos = 0;
    char current;

    while (1) {
        current = source[*pos];
        if (is_alpha_num(current) || current == '_') {
            buf[buf_pos++] = current;
            (*pos)++;
        } else {
            break;
        }
    }

    buf[buf_pos] = '\0';

    Token new_token = create_token();
    unsigned int keyword = match_keyword(buf);

    if (keyword == T_TRUE || keyword == T_FALSE) {
        new_token.type = T_BOOLEAN;
        if (keyword == T_TRUE) {
            new_token.value = malloc(sizeof "true");
            memcpy(new_token.value, "true", sizeof "true");
        }
        if (keyword == T_FALSE) {
            new_token.value = malloc(sizeof "false");
            memcpy(new_token.value, "false", sizeof "false");
        }
    }

    else if (keyword != 0) {
        new_token.type = keyword;
    } else {
        new_token.value = malloc(sizeof(char) * (buf_pos + 1));
        memcpy(new_token.value, buf, buf_pos + 1);
        new_token.type = T_IDENTIFIER;
    }

    return new_token;
}

static Token match_number(char *source, char *buf, unsigned int source_size, unsigned int *pos) {
    unsigned int buf_pos = 0;
    char current;

    while (1) {
        current = source[*pos];
        if (is_num(current) || current == '.') {
            buf[buf_pos++] = current;
            (*pos)++;
        } else {
            break;
        }
    }

    buf[buf_pos] = '\0';

    Token new_token = create_token();
    new_token.value = malloc(sizeof(char) * (buf_pos + 1));
    memcpy(new_token.value, buf, buf_pos + 1);

    new_token.type = T_NUMBER;

    return new_token;
}

static Token match_string(char *source, char *buf, unsigned int source_size, unsigned int *pos) {
    unsigned int buf_pos = 0;
    char current = source[*pos];
    char quote_char = current;

    buf[buf_pos++] = current;
    (*pos)++;

    do {
        current = source[(*pos)++];
        buf[buf_pos++] = current;

        if (*pos > source_size) {
            break; // TODO fix string overflow
        }
    } while (current != quote_char);

    Token new_token = create_token();

    /* excluding the two quote chars */
    buf[buf_pos - 1] = '\0';
    new_token.value = malloc(((buf_pos + 1) - 2) * (sizeof *new_token.value));
    memcpy(new_token.value, buf + 1, buf_pos - 1);

    new_token.type = T_STRING;

    return new_token;
}

static unsigned int match_symbol(char *source, unsigned int source_size, unsigned int *ind) {
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


/* public functions */
TokenArray *tokenize(char *source) {
    unsigned int source_size = strlen(source);
    unsigned int pos = 0;
    char current;

    char char_buf[CHAR_BUF_SZ];

    TokenArray *token_list = create_token_dyn_array();

    while (pos < source_size) {
        current = source[pos];
        if (is_whitespace(current)) {
            pos++; continue;
        }

        if (is_alpha(current)) {
            Token new_token = match_identifier(source, char_buf, source_size, &pos);
            append_to_array(token_list, &new_token);
        }

        else if (is_num(current)) {
            Token new_token = match_number(source, char_buf, source_size, &pos);
            append_to_array(token_list, &new_token);
        }

        else if (is_quote(current)) {
            Token new_token = match_string(source, char_buf, source_size, &pos);
            append_to_array(token_list, &new_token);
        }

        else if (is_symbol(current)) {
            Token new_token = create_token();
            new_token.type = match_symbol(source, source_size, &pos);
            append_to_array(token_list, &new_token);
            pos++;
        }
    }

    return token_list;
}
