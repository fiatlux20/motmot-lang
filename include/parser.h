#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include "bytecode.h"
#include "common.h"
#include "error.h"
#include "tokens.h"
#include "vm.h"

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

typedef struct {
    token_dynamic_array *tokens;
    dynarray_iterator *iter;
    bytecode_array *bytecode;
    unsigned int error;
} parser_state;

bytecode_array parse(virtual_machine *vm, token_dynamic_array *tokens);

#endif /* GRAMMAR_H_ */
