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
 * Pushes a value onto the virtual machine stack and increments the stack head.
 *
 * @param s Pointer to the stack to push to
 * @param val Value to push onto the stack
 */
void push(Stack *s, Value val);

/**
 * Pops and returns a value from the virtual machine stack and decrements the
 * stack head.
 *
 * @param s Pointer to the stack to push to
 * @return Value on top of the stack
 */
Value pop(Stack *s);

/**
 * Takes a virtual machine and an array of bytecode and executes the bytecode,
 * leaving the result on top of the virtual machine's stack.
 *
 * @param vm
 * @param bytecode
 */
void evaluate(VirtualMachine *vm, BytecodeArray *bytecode);

/**
 * Takes a virtual machine and an array of bytecode and executes the bytecode. If
 * there is a value left on the virtual machine's stack it pops and prints the value.
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
