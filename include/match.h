#ifndef _MATCH_H_
#define _MATCH_H_

#include "tokens.h"

unsigned int match_keyword(char *word);
unsigned int match_symbol(char *source, unsigned int source_size, unsigned int *ind);

#endif /* _MATCH_H_ */