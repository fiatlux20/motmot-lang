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
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_UPDATE_GLOBAL,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
} opcode;

typedef struct {
    Value *array;
    uint32_t elements;
    uint32_t capacity;
} ValueArray;

typedef struct {
    char **array;
    uint32_t elements;
    uint32_t capacity;
} NameArray;

typedef struct {
    uint8_t *array;
    NameArray *names;
    ValueArray *constants;
    uint32_t elements;
    uint32_t capacity;
} BytecodeArray;


/* array ops */
BytecodeArray *create_bytecode_dynarray();
void init_bytecode_dynarray(BytecodeArray *array);
void append_to_bytecode_dynarray(BytecodeArray *array, opcode_t op);
opcode_t *next_opcode(BytecodeArray *array, dynarray_iterator *iter);
void free_bytecode_dynarray(BytecodeArray *array);

ValueArray *create_value_dynarray();
void append_to_value_dynarray(ValueArray *array, Value val);
void free_value_dynarray(ValueArray *array);

NameArray create_name_dynarray();
void append_to_name_dynarray(NameArray *array, char *val);
void free_name_dynarray(NameArray *array);

#ifdef DEBUG_VM
void print_disassembly(BytecodeArray *bytecode);
void print_constants(BytecodeArray *bytecode);
void print_names(BytecodeArray *bytecode);
#endif /* DEBUG_VM */

#endif /* _BYTECODE_H_ */
