/** @file vm.h */
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
} Stack;

typedef struct {
    Stack stack;
    NameArray names;
    HashTable *env;
    int ip;
    int state;
} VirtualMachine;

VirtualMachine initialize_vm();

/**
 * Takes a virtual machine and an array of bytecode and executes the bytecode.
 *
 * @param vm
 * @param bytecode
 * @return success
 */
unsigned int execute(VirtualMachine *vm, BytecodeArray *bytecode);
void free_vm(VirtualMachine *vm);

#ifdef DEBUG_VM
void print_disassembly(BytecodeArray *bytecode);
void print_stack(VirtualMachine *vm);
#endif

#endif /* _VM_H_ */
