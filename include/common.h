#ifndef _COMMON_H_
#define _COMMON_H_

#define DEBUG_VM
#define DEBUG_STACK
#undef DEBUG_TOKENS
#undef DEBUG_PARSER
#define DEBUG_COMPILER
#define DEBUG_TABLE

#define INPUT_BUFFER_SIZE 1024
#define STACK_SIZE 1024

#define MAX_CHUNK_CONSTANTS 256
#define MAX_CHUNK_NAMES 256

#define DYNARRAY_INITIAL_SIZE 8
#define DYNARRAY_GROW_BY_FACTOR 2
typedef struct {
    unsigned int array_capacity;
    unsigned int index;
} dynarray_iterator;

#endif /* _COMMON_H_ */
