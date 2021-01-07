/** \file parser.h
 * Contains functions related to parsing expressions from an array of tokens
 */
#ifndef _JITCALC_PARSER_H_
#define _JITCALC_PARSER_H_

typedef struct token Token;
typedef struct token_array TokenArray;
typedef struct function_table FunctionTable;

typedef struct parser ParserState;
typedef void (*ParseFn)(ParserState*);

typedef enum {
    PREC_NONE,
    PREC_NUMBER,
    PREC_EQUALITY,
    PREC_TERM,
    PREC_FACTOR,
    PREC_CALL,
    PREC_DEFUN
} Precedence;

typedef enum {
    I_MOV,
    I_MOV_STACK,
    I_MOVAPS,
    I_ADD,
    I_SUB,
    I_MUL,
    I_DIV,
    I_SQRT,
    I_XOR,
    I_ROUND,
    I_AND,
    I_CMP,
    I_COMISS
} Instruction;

typedef struct rule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} Rule;

typedef struct byte_array {
    unsigned char *array;
    unsigned int elements;
    unsigned int capacity;
} ByteArray;

struct parser {
    float *constants;
    unsigned int *flags;
    FunctionTable *functions;
    Token *current;
    ByteArray *code;
    unsigned int constants_ind;
    unsigned int current_reg : 30;
    unsigned char error : 1;
    unsigned char parse_function : 1;
};

typedef struct {
    float *constants;
    ByteArray *code;
} Chunk;

/**
 * Returns the parser state after parsing an array of tokens which includes
 *
 * @param tokens The array of tokens to be parsed
 * @param functions 
 */
ParserState *parse(TokenArray *tokens, FunctionTable *functions);

/**
 * Frees all heap allocated memory used by the Parser struct and sets the
 * pointer to NULL
 *
 * @param parser The pointer to the parser state to free. Set to NULL after
 *               the function returns.
 */
void free_parser(ParserState *parser);

#ifdef DEBUG
void disassemble(ByteArray *code);
#endif

#endif /* _JITCALC_PARSER_H_ */
