/** @file parser.h
 *
 */
#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include "bytecode.h"
#include "common.h"
#include "error.h"
#include "tokens.h"
#include "vm.h"

typedef struct ParserState {
    Token *current;
    Token *prev;
    ArrayIterator *iter;
    BytecodeArray *bytecode;
    unsigned int error;
} ParserState;

typedef void (*ParsingFunction)(ParserState* s);

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_TERM,
    PREC_FACTOR
} Precedence;

typedef struct Rule {
    ParsingFunction prefix;
    ParsingFunction infix;
    Precedence precedence;
} Rule;

/**
 * Takes a pointer to the virtual machine and an array of tokens and returns an
 * array of bytecode to be run by the virtual machine. The reference to the
 * virtual machine is so that the parser can include bytecode which references
 * functions or global variables.
 *
 * The parser is a
 * <a href="https://en.wikipedia.org/wiki/Operator-precedence_parser#Pratt_parsing">Pratt parser</a>
 * which uses operator precedence referenced in a table to correctly parse operator precedence
 * and overall simplify the parsing code versus a recursive descent parser.
 *
 * @param vm A pointer to a virtual machine which includes global variables
 *           which can be referenced by the parser
 * @param tokens A token array to parse and compile into bytecode
 * @return An array of bytecode which can be run with execute()
 */
BytecodeArray *parse(VirtualMachine *vm, TokenArray *tokens);

#endif /* GRAMMAR_H_ */
