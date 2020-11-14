#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#define CHAR_BUF_SZ 1024

typedef struct token {
    char *value;
    unsigned int type;
} token;

typedef struct linked_token_list {
    token *node;
    struct linked_token_list *next;
} linked_token_list;

linked_token_list *tokenize(char *source);
void print_tokens(linked_token_list *list);
void free_tokens(linked_token_list *list);

#endif /* _TOKENIZE_H_ */
