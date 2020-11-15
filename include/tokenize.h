#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "tokens.h"

#define CHAR_BUF_SZ 1024

linked_token_list *tokenize(char *source);
void print_tokens(linked_token_list *list);
void free_tokens(linked_token_list *list);

#endif /* _TOKENIZE_H_ */
