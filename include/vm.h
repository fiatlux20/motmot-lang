#ifndef _VM_H_
#define _VM_H_

#include "bytecode.h"
#include "common.h"
#include "error.h"
#include "table.h"

#define BINARY_OP(op)     \
    do {                  \
        double b = pop(); \
        double a = pop(); \
        push (a op b);    \
    } while (0)

typedef struct {
    unsigned int head;
    unsigned int size;
    Value *at;
} vm_stack;

typedef struct {
    vm_stack stack;
    name_array names;
    HashTable *env;
    int ip;
    int state;
} virtual_machine;

virtual_machine initialize_vm();
unsigned int execute(virtual_machine *vm, bytecode_array *bytecode);
void free_vm(virtual_machine *vm);

#ifdef DEBUG_VM
void print_disassembly(bytecode_array *bytecode);
void print_stack(virtual_machine *vm);
#endif

#endif /* _VM_H_ */
