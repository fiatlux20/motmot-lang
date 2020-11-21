#include <stdio.h>
#include <stdlib.h>

#include "bytecode.h"

static void grow_bytecode_array(bytecode_array *array, unsigned int new_size) {
    array->array = realloc(array->array, new_size);
    if (array->array == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }
}

static void grow_value_array(value_array *array, unsigned int new_size) {
    if (new_size > MAX_CHUNK_CONSTANTS) {
        fputs("error: constants overflow\n", stderr);
        return;
    }

    array->array = realloc(array->array, new_size * sizeof(value));
    if (array->array == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }
}

value create_number(double n) {
    value val;
    val.type = T_NUMBER;
    val.as.d = n;
    return val;
}

value_array create_value_dynarray() {
    value_array array;
    array.elements = 0;
    array.capacity = DYNARRAY_INITIAL_SIZE;
    array.array = malloc(sizeof(value) * DYNARRAY_INITIAL_SIZE);
    return array;
}

void append_to_value_dynarray(value_array *array, value val) {
    if (array->elements == array->capacity) {
        array->capacity *= DYNARRAY_GROW_BY_FACTOR;
        grow_value_array(array, array->capacity);

        if (array->array == NULL) {
            printf("got NULL while performing array realloc\n");
            return;
        }
    }

    array->array[array->elements++] = val;
}

void free_value_dynarray(value_array *array) {
    free(array->array);
    array->array = NULL;
}

bytecode_array create_bytecode_dynarray() {
    bytecode_array array;
    array.elements = 0;
    array.capacity = DYNARRAY_INITIAL_SIZE;
    array.array = malloc(sizeof(uint8_t) * DYNARRAY_INITIAL_SIZE);
    array.constants = create_value_dynarray();
    return array;
}

void append_to_bytecode_dynarray(bytecode_array *array, opcode_t op) {
    if (array->elements == array->capacity) {
        array->capacity *= DYNARRAY_GROW_BY_FACTOR;
        grow_bytecode_array(array, array->capacity);

        if (array->array == NULL) {
            printf("got NULL while performing array realloc\n");
            return;
        }
    }

    array->array[array->elements++] = op;
}

opcode_t *next_opcode(bytecode_array *array, dynarray_iterator *iter) {
    if (iter->index + 1 >= array->elements) {
        return NULL;
    }

    if (iter->index < array->elements) {
        iter->index++;
    }

    return &(array->array[iter->index]);
}

void free_bytecode_dynarray(bytecode_array *array) {
    free_value_dynarray(&array->constants);
    free(array->array);
    array->array = NULL;
}

#ifdef DEBUG_COMPILER
static void print_opcode(opcode_t opcode) {
    switch(opcode) {
    case OP_RETURN: printf("%02x RETURN\n", opcode); break;
    case OP_CONSTANT: printf("%02x CONSTANT\n", opcode); break;
    case OP_ADD: printf("%02x ADD\n", opcode); break;
    case OP_SUB: printf("%02x SUB\n", opcode); break;
    case OP_MULT: printf("%02x MULT\n", opcode); break;
    case OP_DIV: printf("%02x DIV\n", opcode); break;
    default: printf("%02x UNKNOWN\n", opcode); break;
    }
}

// void print_disassembly(bytecode_array *bytecode) {
//     opcode_t *opcode;
//     dynarray_iterator iter = { bytecode->elements, 0 };

//     fputs("--- Begin disassembly ---\n", stdout);
//     while ((opcode = next_opcode(bytecode, &iter)) != NULL) {
//         if (*opcode == OP_CONSTANT) {
//             print_opcode(*opcode);
//             uint8_t *constant_index = next_opcode(bytecode, &iter);
//             printf("%lf\n", bytecode->constants.array[*constant_index].d);
//         }
//         print_opcode(*opcode);
//     }
// }

void print_disassembly(bytecode_array *bytecode) {
    unsigned int length = bytecode->elements;
    unsigned int constant_index = 0;

    fputs("---- disassembly ----\n", stdout);
    for (unsigned int i = 0; i < length; i++) {
        opcode_t op = bytecode->array[i];
        printf("%04d  ", i);
        if (op == OP_CONSTANT) {
            printf("%02x CONSTANT (%lf)\n", op, bytecode->constants.array[constant_index++].as.d);
            i += 1;
        } else {
            print_opcode(bytecode->array[i]);
        }
    }
}
#endif /* DEBUG_COMPILER */