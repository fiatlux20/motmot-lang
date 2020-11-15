#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "tokens.h"

#define CHAR_BUF_SZ 1024

token_dynamic_array *tokenize(char *source);
void print_tokens(token_dynamic_array *array);

#endif /* _TOKENIZE_H_ */
