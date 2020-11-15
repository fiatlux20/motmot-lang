#include <stdio.h>
#include <stdlib.h>
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

int is_symbol(char c) {
    return (c >= 33 && c <= 47) || (c >=58 && c <= 64) || (c >= 91 && c <= 96) || (c >=123 && c <=126);
}

/* token ops */
linked_token_list *create_token_list() {
    linked_token_list *ltl = malloc(sizeof(linked_token_list));

    ltl->next = NULL;
    ltl->node = NULL;

    return ltl;
}

void append_to_list(linked_token_list *list, token *node) {
    if (list->node == NULL) {
        list->node = node;
    } else if (list->next == NULL) {
        list->next = create_token_list();
        append_to_list(list->next, node);
    } else {
        append_to_list(list->next, node);
    }
}

token *create_token() {
    token *t = malloc(sizeof(token));

    t->value = NULL;
    t->type = T_NONE;

    return t;
}

void free_token(token *t) {
    if (t == NULL) {
        return;
    }

    if (t->value != NULL) {
        free(t->value);
        t->value = NULL;
    }

    free(t);
    t = NULL;
}

void print_token(token *token) {
    if (token->value != NULL) {
        if (token->type == T_NUMBER) {
            printf("(NUMBER, '%s')\n", token->value);
        } else if (token->type == T_IDENTIFIER) {
            printf("(IDENTIFIER, '%s')\n", token->value);
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

        case T_EQL: symbol = "EQL"; break;
        case T_DBL_EQL: symbol = "DBL_EQL"; break;
        case T_BANG: symbol = "BANG"; break;
        case T_BANG_EQL: symbol = "BANG_EQL"; break;
        default:
            symbol = "UNDEF";
        }

        printf("(%s, %d)\n", symbol, token->type);
    }
}


/* public functions */
linked_token_list *tokenize(char *source) {
    unsigned int source_size = strlen(source);

    unsigned int in_word = 0;
    unsigned int in_number = 0;

    char char_buf[CHAR_BUF_SZ];
    unsigned int char_buf_ind = 0;

    linked_token_list *token_list = create_token_list();

    for (unsigned int ind = 0; ind < source_size; ind++) {
        char current = source[ind];

        if (!in_word && !in_number && is_whitespace(current)) {
            continue;
        }

        if (!in_word && !in_number && is_num(current)) {
            in_number = 1;
            char_buf_ind = 0;
        } else if (!in_word && !in_number && is_alpha(current)) {
            in_word = 1;
            char_buf_ind = 0;
        }

        if (in_number && !in_word && (current == '.' || is_num(current))) {
            char_buf[char_buf_ind++] = current;
        } else if (in_number && !in_word && !(current == '.' || is_num(current))) {
            char_buf[char_buf_ind] = '\0';

            token *new_token = create_token();
            new_token->value = malloc(sizeof(char) * (char_buf_ind + 1));
            new_token->type = T_NUMBER;
            memcpy(new_token->value, char_buf, char_buf_ind + 1);

            append_to_list(token_list, new_token);
            in_number = 0;
        }

        if (in_word && !in_number && is_alpha_num(current)) {
            char_buf[char_buf_ind++] = current;
        } else if (in_word && !in_number && !is_alpha_num(current)) {
            char_buf[char_buf_ind] = '\0';

            token *new_token = create_token();

            if ((new_token->type = match_keyword(char_buf)) == 0) {
                new_token->value = malloc(sizeof(char) * (char_buf_ind + 1));
                new_token->type = T_IDENTIFIER;
                memcpy(new_token->value, char_buf, char_buf_ind + 1);
            }

            append_to_list(token_list, new_token);
            in_word = 0;
        }

        if (!in_word && !in_number && is_symbol(current)) {
            token *new_token = create_token();
            new_token->type = match_symbol(source, source_size, &ind);
            append_to_list(token_list, new_token);
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
