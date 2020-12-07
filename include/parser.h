#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include "bytecode.h"
#include "common.h"
#include "error.h"
#include "tokens.h"
#include "vm.h"

typedef struct rule Rule;
typedef struct parser_state ParserState;
typedef void (*ParsingFunction)(ParserState* s);

struct parser_state {
    Token *current;
    Token *prev;
    dynarray_iterator *iter;
    BytecodeArray *bytecode;
    unsigned int error;
};

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_TERM,
    PREC_FACTOR
} Precedence;

struct rule {
    ParsingFunction prefix;
    ParsingFunction infix;
    Precedence precedence;
};

BytecodeArray *parse(VirtualMachine *vm, TokenArray *tokens);

#endif /* GRAMMAR_H_ */
