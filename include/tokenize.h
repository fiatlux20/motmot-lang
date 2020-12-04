#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "tokens.h"

#define CHAR_BUF_SZ 1024

TokenArray *tokenize(char *source);
void print_tokens(TokenArray *array);

#endif /* _TOKENIZE_H_ */
