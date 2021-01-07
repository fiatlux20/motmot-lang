#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <unistd.h>

#include "parser.h"
#include "table.h"
#include "function.h"

static void resize_key_array(KeyArray *array, size_t new_size);
static void push_key(KeyArray *array, char *key, unsigned int length);
static void free_executable_memory(FunctionTable *ft);


KeyArray *create_key_array() {
    KeyArray *keys = malloc(sizeof *keys);
    keys->capacity = 8;
    keys->elements = 0;
    keys->keys = malloc(sizeof *keys->keys * keys->capacity);
    return keys;
}

void move_key(KeyArray *array, char *key) {
    if (array->elements >= array->capacity) {
        resize_key_array(array, array->capacity * 2);
    }

    array->keys[array->elements++] = key;
}

void free_keys_array(KeyArray *array) {
    for (int i = 0; i < array->elements; i++) {
        free(array->keys[i]);
    }

    free(array->keys);
    free(array);
    array = NULL;
}

FunctionTable *create_function_table() {
    FunctionTable *ft = malloc(sizeof *ft);
    ft->memory_size = ALLOC_SIZE;
    ft->executable_memory = mmap(NULL, ft->memory_size,
         PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ft->keys = create_key_array();
    ft->table = create_hash_table();

    static const uint32_t zero = 0x00000000;
    static const uint32_t one = 0x3f800000;
    static const uint32_t sign_bit = 0x80000000;

    ft->zero_offset = 0;
    ft->one_offset  = 4;
    ft->sign_offset = 8;
    ((uint32_t *)ft->executable_memory)[ft->zero_offset / sizeof(uint32_t)] = zero;
    ((uint32_t *)ft->executable_memory)[ft->one_offset /  sizeof(uint32_t)] = one;
    ((uint32_t *)ft->executable_memory)[ft->sign_offset / sizeof(uint32_t)] = sign_bit;

    ft->bytes_occupied = sizeof(uint32_t) * 3;
    return ft;
}

Function *make_function(FunctionDef function, unsigned int memory_offset) {
    Function *f = malloc(sizeof *f);
    f->func = function;
    f->offset = memory_offset;
    return f;
}

float (*compile_expression(FunctionTable *functions, ByteArray *code))(float*) {
    void *location = (void*) ((char*) functions->executable_memory + functions->bytes_occupied);

    memcpy(location, code->array, code->elements);
    return (float (*)(float*)) (void*) ((char*) location);
}

float (*compile_function(FunctionTable *functions, ByteArray *code))(float, float*) {
    void *location = (void*) ((char*) functions->executable_memory + functions->bytes_occupied);

    memcpy(location, code->array, code->elements);
    functions->bytes_occupied += code->elements;
    return (float (*)(float, float*)) (void*) ((char*) location);
}

void free_function_table(FunctionTable *ft) {
    free_hash_table(ft->table);
    free_executable_memory(ft);
    free_keys_array(ft->keys);
    free(ft);
    ft = NULL;
}

void dump_memory(FunctionTable *functions) {
    char *reader = (char*) functions->executable_memory;
    for (int i = 0; i < functions->memory_size; i++) {
        if (i > 0 && i % 16 == 0) {
            printf("\n");
        }
        printf("%02x ", (unsigned char) *(reader++));
    }
    printf("\n");
}

void dump_memory_range(FunctionTable *functions, unsigned long low, unsigned long high) {
    unsigned char *reader = ((unsigned char*) functions->executable_memory) + low;
    for (int i = 0; i < high - low; i++) {
        if (i > 0 && i % 16 == 0) {
            printf("\n");
        }
        printf("%02x ", *(reader++));
    }
    printf("\n");
}

void dump_memory_range_highlight(FunctionTable *functions, unsigned long low, unsigned long high, unsigned long byte) {
    unsigned char *reader = ((unsigned char*) functions->executable_memory) + low;
    for (int i = 0; i < high - low; i++) {
        if (i > 0 && i % 16 == 0) {
            printf("\n");
        }
        if (i == byte) {
            printf("\033[4m%02x\033[0m ", *(reader++));
        } else {
            printf("%02x ", *(reader++));
        }
    }
    printf("\n");
}

static void free_executable_memory(FunctionTable *ft) {
    munmap(ft->executable_memory, ft->memory_size);
}

static void resize_key_array(KeyArray *array, size_t new_size) {
    char **old = array->keys;
    array->keys = malloc((sizeof *array->keys) * new_size);
    array->capacity = new_size;

    for (int i = 0; i < array->elements; i++) {
        move_key(array, old[i]);
    }

    free(old);
}

static void push_key(KeyArray *array, char *key, unsigned int length) {
    if (array->elements >= array->capacity) {
        resize_key_array(array, array->capacity * 2);
    }

    array->keys[array->elements++] = strndup(key, length);
}
