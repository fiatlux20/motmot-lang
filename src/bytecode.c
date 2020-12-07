#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode.h"

static void grow_bytecode_array(BytecodeArray *array, unsigned int new_size) {
    array->array = realloc(array->array, new_size);
    if (array->array == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }
}

/* value array */
static void grow_value_array(ValueArray *array, unsigned int new_size) {
    if (new_size > MAX_CHUNK_CONSTANTS) {
        fputs("error: constants overflow\n", stderr);
        return;
    }

    array->array = realloc(array->array, new_size * (sizeof *array->array));
    if (array->array == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }
}

ValueArray *create_value_dynarray() {
    ValueArray *array = malloc(sizeof *array);
    array->elements = 0;
    array->capacity = DYNARRAY_INITIAL_SIZE;
    array->array = malloc(DYNARRAY_INITIAL_SIZE * (sizeof *array->array));
    return array;
}

void append_to_value_dynarray(ValueArray *array, Value val) {
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

void free_value_dynarray(ValueArray *array) {
    free(array->array);
    array->array = NULL;
    free(array);
    array = NULL;
}

/* names array */
static void grow_name_array(NameArray *array, unsigned int new_size) {
    if (new_size > MAX_CHUNK_NAMES) {
        fputs("error: names overflow\n", stderr);
        return;
    }

    char **old_array = array->array;
    array->array = calloc(sizeof *array->array, new_size);
    if (array->array == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }

    for (unsigned int i = 0; i < array->elements; i++) {
        array->array[i] = old_array[i];
    }

    free(old_array);
}

NameArray create_name_dynarray() {
    NameArray array;
    array.elements = 0;
    array.capacity = DYNARRAY_INITIAL_SIZE;
    array.array = calloc(DYNARRAY_INITIAL_SIZE, (sizeof *array.array));
    return array;
}

void append_to_name_dynarray(NameArray *array, char *val) {
    if (array->elements == array->capacity) {
        array->capacity *= DYNARRAY_GROW_BY_FACTOR;
        grow_name_array(array, array->capacity);

        if (array->array == NULL) {
            printf("got NULL while performing array realloc\n");
            return;
        }
    }

    unsigned int len = strlen(val);
    array->array[array->elements] = malloc(len + 1);
    memcpy(array->array[array->elements], val, len);
    array->array[array->elements][len] = '\0';
    array->elements++;
}

void free_name_dynarray(NameArray *array) {
    for (unsigned int i = 0; i < array->elements; i++) {
        free(array->array[i]);
        array->array[i] = NULL;
    }

    free(array->array);
    array->array = NULL;
}

/* bytecode chunk */
BytecodeArray *create_bytecode_dynarray() {
    BytecodeArray *array = malloc(sizeof *array);
    array->elements = 0;
    array->capacity = DYNARRAY_INITIAL_SIZE;
    array->array = malloc(sizeof(uint8_t) * DYNARRAY_INITIAL_SIZE);
    array->constants = create_value_dynarray();
    array->names = NULL;
    // array.names = create_name_dynarray(); // handled by vm
    return array;
}

void append_to_bytecode_dynarray(BytecodeArray *array, opcode_t op) {
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

opcode_t *next_opcode(BytecodeArray *array, dynarray_iterator *iter) {
    if (iter->index + 1 >= array->elements) {
        return NULL;
    }

    if (iter->index < array->elements) {
        iter->index++;
    }

    return &(array->array[iter->index]);
}

void free_bytecode_dynarray(BytecodeArray *array) {
    free_value_dynarray(array->constants);
    // free_name_dynarray(&array->names); // handled by vm
    free(array->array);
    array->array = NULL;

    free(array);
    array = NULL;
}

#ifdef DEBUG_COMPILER
static void print_opcode(opcode_t opcode) {
    switch(opcode) {
    case OP_RETURN: printf("%02x RETURN\n", opcode); break;
    case OP_CONSTANT: printf("%02x CONSTANT\n", opcode); break;
    case OP_GET_GLOBAL: printf("%02x GET_GLOBAL\n", opcode); break;
    case OP_SET_GLOBAL: printf("%02x SET_GLOBAL\n", opcode); break;
    case OP_ADD: printf("%02x ADD\n", opcode); break;
    case OP_SUB: printf("%02x SUB\n", opcode); break;
    case OP_MULT: printf("%02x MULT\n", opcode); break;
    case OP_DIV: printf("%02x DIV\n", opcode); break;
    default: printf("%02x UNKNOWN\n", opcode); break;
    }
}

// void print_disassembly(BytecodeArray *bytecode) {
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

void print_disassembly(BytecodeArray *bytecode) {
    unsigned int length = bytecode->elements;
    Value *v;

    fputs("---- disassembly ----\n", stdout);
    for (unsigned int i = 0; i < length; i++) {
        opcode_t op = bytecode->array[i];
        printf("%04d  ", i);
        switch (op) {
        case OP_CONSTANT:
            v = &bytecode->constants->array[bytecode->array[i + 1]];
            printf("%02x CONSTANT (", op);
            print_value(v);
            printf(")\n");
            i++;
            break;
        case OP_GET_GLOBAL:
            printf("%02x GET_GLOBAL (%s)\n", op, bytecode->names->array[bytecode->array[i + 1]]);
            i++;
            break;
        case OP_UPDATE_GLOBAL:
            printf("%02x UPDATE_GLOBAL (%s)\n", op, bytecode->names->array[bytecode->array[i + 1]]);
            i++;
            break;
        case OP_SET_GLOBAL:
            printf("%02x SET_GLOBAL (%s)\n", op, bytecode->names->array[bytecode->array[i + 1]]);
            i++;
            break;
        default:
            print_opcode(bytecode->array[i]);
        }
    }
}

void print_constants(BytecodeArray *bytecode) {
    fputs("---- constants ----\n", stdout);

    for (unsigned int i = 0; i < bytecode->constants->capacity; i++) {
        printf("%d: ", i);
        print_value(&bytecode->constants->array[i]);
        printf("\n");
    }
}

void print_names(BytecodeArray *bytecode) {
    fputs("---- names ----\n", stdout);

    printf("# elements = %d\n", bytecode->names->elements);
    for (unsigned int i = 0; i < bytecode->names->capacity; i++) {
        if (bytecode->names->array[i] != NULL) {
            printf("%d: %s\n", i, bytecode->names->array[i]);
        }
    }
}
#endif /* DEBUG_COMPILER */
