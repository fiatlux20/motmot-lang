#ifndef _BYTECODE_H_
#define _BYTECODE_H_

#include <stdint.h>

#include "common.h"
#include "tokens.h"
#include "value.h"

typedef unsigned char opcode_t;

typedef enum {
    OP_RETURN,
    OP_CONSTANT,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
} opcode;

typedef struct {
    Value *array;
    uint32_t elements;
    uint32_t capacity;
} value_array;

typedef struct {
    uint8_t *array;
    value_array constants;
    uint32_t elements;
    uint32_t capacity;
} bytecode_array;


/* array ops */
bytecode_array create_bytecode_dynarray();
void init_bytecode_dynarray(bytecode_array *array);
void append_to_bytecode_dynarray(bytecode_array *array, opcode_t op);
opcode_t *next_opcode(bytecode_array *array, dynarray_iterator *iter);
void free_bytecode_dynarray(bytecode_array *array);

value_array create_value_dynarray();
void append_to_value_dynarray(value_array *array, Value val);
void free_value_dynarray(value_array *array);

#ifdef DEBUG_VM
void print_disassembly(bytecode_array *bytecode);
#endif /* DEBUG_VM */

#endif /* _BYTECODE_H_ */
