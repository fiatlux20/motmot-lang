/** @file tokenize.h */
#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "tokens.h"

#define CHAR_BUF_SZ 1024

/**
 * Takes a source code string and breaks it into tokens which represent
 * symbols or keywords in the language to be validated and parsed by parse().
 *
 * @param source A source code string to be tokenized
 * @return An array of tokens generated from the source code
 */
TokenArray *tokenize(char *source);

/**
 * Prints out all of the tokens in a token array
 *
 * @param array The TokenArray to print out
 */
void print_tokens(TokenArray *array);

#endif /* _TOKENIZE_H_ */
