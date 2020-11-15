#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tokenize.h"


/* util */
int is_alpha(char c) {
    return ((c >= 97 && c <= 122) || (c >= 65 && c <= 90));
}

int is_num(char c) {
    return (c >= 48 && c <= 57);
}

int is_alpha_num(char c) {
    return ((c >= 97 && c <= 122) || (c >= 48 && c <= 57) || (c >= 65 && c <= 90));
}

int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n');
}

int is_quote(char c) {
    return c == 39 || c == 34;
}

int is_symbol(char c) {
    if (is_quote(c)) {
        return 0;
    }
    return (c >= 33 && c <= 47) || (c >=58 && c <= 64) || (c >= 91 && c <= 96) || (c >=123 && c <=126);
}


/* token matching functions */
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

token *match_identifier(char *source, char *buf, unsigned int source_size, unsigned int *pos) {
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

    token *new_token = create_token();
    unsigned int keyword = match_keyword(buf);

    if (keyword != 0) {
        new_token->type = keyword;
    } else {
        new_token->value = malloc(sizeof(char) * (buf_pos + 1));
        memcpy(new_token->value, buf, buf_pos + 1);
        new_token->type = T_IDENTIFIER;
    }

    return new_token;
}

token *match_number(char *source, char *buf, unsigned int source_size, unsigned int *pos) {
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

    token *new_token = create_token();
    new_token->value = malloc(sizeof(char) * (buf_pos + 1));
    memcpy(new_token->value, buf, buf_pos + 1);

    new_token->type = T_NUMBER;

    return new_token;
}

token *match_string(char *source, char *buf, unsigned int source_size, unsigned int *pos) {
    unsigned int buf_pos = 0;
    char current = source[*pos];
    char quote_char = current;

    buf[buf_pos++] = current;
    (*pos)++;

    do {
        current = source[(*pos)++];
        buf[buf_pos++] = current;

        if (*pos > source_size) {
            return NULL;
        }
    } while (current != quote_char);

    buf[buf_pos] = '\0';

    token *new_token = create_token();

    new_token->value = malloc(sizeof(char) * (buf_pos + 1));
    memcpy(new_token->value, buf, buf_pos + 1);

    new_token->type = T_STRING;

    return new_token;
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

void print_token(token *token) {
    if (token == NULL) {
        printf("NULL TOKEN\n");
        return;
    }

    if (token->value != NULL) {
        switch (token->type) {
        case T_NUMBER: { printf("(NUMBER, '%s')\n", token->value); break; } 
        case T_IDENTIFIER: { printf("(IDENTIFIER, '%s')\n", token->value); break; }
        case T_STRING: { printf("(STRING, '%s')\n", token->value); break; }
        default: break;
        }
    } else {
        const char *symbol;

        switch(token->type) {
        case T_AND: symbol = "AND"; break;
        case T_OR: symbol = "OR"; break;
        case T_FUN: symbol = "FUN"; break;
        case T_IF: symbol = "IF"; break;
        case T_ELSE: symbol = "ELSE"; break;
        case T_FOR: symbol = "FOR"; break;
        case T_WHILE: symbol = "WHILE"; break;

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


/* public functions */
linked_token_list *tokenize(char *source) {
    unsigned int source_size = strlen(source);
    unsigned int pos = 0;
    char current;

    char char_buf[CHAR_BUF_SZ];

    linked_token_list *token_list = create_token_list();

    while (pos < source_size) {
        current = source[pos];
        if (is_whitespace(current)) {
            pos++; continue;
        }

        if (is_alpha(current)) {
            token *new_token = match_identifier(source, char_buf, source_size, &pos);
            if (new_token == NULL) {
                fprintf(stderr, "error unable to parse identifier\n");
                continue;
            } else {
                append_to_list(token_list, new_token);
            }
        }

        else if (is_num(current)) {
            token *new_token = match_number(source, char_buf, source_size, &pos);
            if (new_token == NULL) {
                fprintf(stderr, "error: unable to parse number\n");
                continue;
            } else {
                append_to_list(token_list, new_token);
            }
        }

        else if (is_quote(current)) {
            token *new_token = match_string(source, char_buf, source_size, &pos);
            if (new_token == NULL) {
                fprintf(stderr, "error: unable to parse string\n");
                continue;
            } else {
                append_to_list(token_list, new_token);
            }
        }

        else if (is_symbol(current)) {
            token *new_token = create_token();
            new_token->type = match_symbol(source, source_size, &pos);
            append_to_list(token_list, new_token);
            pos++;
        }
    }

    return token_list;
}

void print_tokens(linked_token_list *list) {
    if (list->node != NULL) {
        print_token(list->node);
    }

    if (list->next != NULL) {
        print_tokens(list->next);
    }
}

void free_tokens(linked_token_list *list) {
    if (list->node != NULL) {
        free_token(list->node);
    }

    if (list->next != NULL) {
        free_tokens(list->next);
    }

    free(list);
    list = NULL;
}
