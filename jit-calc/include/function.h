#ifndef _JITCALC_FUNCTION_H_
#define _JITCALC_FUNCTION_H_

#define ALLOC_SIZE 1024

typedef struct byte_array ByteArray;
typedef struct hash_table HashTable;
typedef float (*FunctionDef) (float, float*);

typedef struct function {
    FunctionDef func;
    unsigned int offset;
} Function;

typedef struct key_array {
    char **keys;
    unsigned int elements;
    unsigned int capacity;
} KeyArray;

typedef struct function_table {
    void *executable_memory;
    KeyArray *keys;
    HashTable *table;
    unsigned int memory_size;
    unsigned int bytes_occupied;
    unsigned int zero_offset;
    unsigned int one_offset;
    unsigned int sign_offset;
} FunctionTable;

KeyArray *create_key_array();
void move_key(KeyArray *array, char *key);
void free_keys_array(KeyArray *array);

Function *make_function(FunctionDef func, unsigned int memory_offset);

FunctionTable *create_function_table();
void free_function_table(FunctionTable *table);
void dump_memory(FunctionTable *functions);
void dump_memory_range(FunctionTable *functions, unsigned long low, unsigned long high);

float (*compile_expression(FunctionTable *functions, ByteArray *code))(float*);
float (*compile_function(FunctionTable *functions, ByteArray *code))(float, float*);

#endif /* _JITCALC_FUNCTION_H_ */
