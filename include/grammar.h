#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include "bytecode.h"
#include "tokens.h"

/*
  expression     → equality ;
  equality       → comparison ( ( "!=" | "==" ) comparison )* ;
  comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
  term           → factor ( ( "-" | "+" ) factor )* ;
  factor         → unary ( ( "/" | "*" ) unary )* ;
  unary          → ( "!" | "-" ) unary
                 | primary ;
  primary        → NUMBER | STRING | "true" | "false" | "nil"
                 | "(" expression ")" ;
 */

/*
typedef union {
    long integer;
    double number;
    char *string;
    char boolean;
} literal;

typedef union {
    expression *expr;
    literal value;
} expr;

typedef struct {

} parse_tree;

typedef struct expression {
    struct expression *left;
    unsigned int op;
    struct expression *right;
} expression;

typedef struct parse_tree_node {
    struct parse_tree_node **next;
    expression;
} parse_tree_node;
*/

bytecode_array parse(token_dynamic_array *tokens);

#endif /* GRAMMAR_H_ */
