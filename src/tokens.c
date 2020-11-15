#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"


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