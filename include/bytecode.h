/** @file bytecode.h */
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
    OP_CMP,
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

/**
 * Allocates a BytecodeArray on the heap and initializes its values to defaults.
 * Must be freed with free_bytecode_dynarray().
 *
 * @return A pointer to a default initialized BytecodeArray.
 */
BytecodeArray *create_bytecode_dynarray();

/**
 * Resizes the array if necessary and then adds an opcode to the array.
 *
 * @param array The Array to append to.
 * @param op The opcode to append to the array.
 */
void append_to_bytecode_dynarray(BytecodeArray *array, opcode_t op);

/**
 * Returns the next opcode in the array if iterating through it using a
 * dynamic array iterator.
 *
 * @param array The array being iterated through.
 * @param iter The iterator being used on the array.
 * @return The next opcode in the array.
 */
opcode_t *next_opcode(BytecodeArray *array, ArrayIterator *iter);

/**
 * Frees a heap-allocated bytecode array and sets the pointer to NULL.
 *
 * @param array Pointer to the array to be freed.
 */
void free_bytecode_dynarray(BytecodeArray *array);


/**
 * Allocates a ValueArray on the heap and initializes its values to defaults.
 * Must be freed with free_value_dynarray().
 *
 * @return A pointer to a default initialized ValueArray.
 */
ValueArray *create_value_dynarray();

/**
 * Resizes the array if necessary and then adds an value to the array.
 *
 * @param array The Array to append to.
 * @param val The value to append to the array.
 */
void append_to_value_dynarray(ValueArray *array, Value val);

/**
 * Frees a heap-allocated value array and sets the pointer to NULL.
 *
 * @param array Pointer to the array to be freed.
 */
void free_value_dynarray(ValueArray *array);

/**
 * Returns a NameArray struct. Names added to the struct must be freed with
 * free_name_dynarray().
 *
 * @return A default initialized NameArray.
 */
NameArray create_name_dynarray();

/**
 * Resizes the array if necessary and then adds a string to the array.
 *
 * @param array The Array to append to.
 * @param val The string to append to the array.
 */
void append_to_name_dynarray(NameArray *array, char *val);

/**
 * Frees strings added to a name array.
 *
 * @param array Pointer to the array to free.
 */
void free_name_dynarray(NameArray *array);

#ifdef DEBUG_VM
void print_disassembly(BytecodeArray *bytecode);
void print_constants(BytecodeArray *bytecode);
void print_names(BytecodeArray *bytecode);
#endif /* DEBUG_VM */

#endif /* _BYTECODE_H_ */
