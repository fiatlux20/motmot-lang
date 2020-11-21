#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vm.h"

static void push(vm_stack *s, value val) {
    if (s->head >= STACK_SIZE) {
        s->head = 0;
    }

#ifdef DEBUG_STACK
    printf("pushing %lf. stack head at %d\n", val.as.d, s->head);
#endif
    s->size++;
    s->at[s->head++] = val;
}

static value pop(vm_stack *s) {
    if (s->head < 0) {
        s->head = STACK_SIZE - 1;
    }

    s->size--;
#ifdef DEBUG_STACK
    value val = s->at[--s->head];
    printf("popping %lf. stack head at %d\n", val.as.d, s->head);

    return val;
#else
    return s->at[--s->head];
#endif
}

/* opcodes */
static void op_add(vm_stack *s) {
    value a = pop(s);
    value b = pop(s);

    push(s, create_number(a.as.d + b.as.d));
}

static void op_sub(vm_stack *s) {
    value a = pop(s);
    value b = pop(s);

    push(s, create_number(a.as.d - b.as.d));
}

static void op_mult(vm_stack *s) {
    value a = pop(s);
    value b = pop(s);

    push(s, create_number(a.as.d * b.as.d));
}

static void op_div(vm_stack *s) {
    value a = pop(s);
    value b = pop(s);

    push(s, create_number(b.as.d / a.as.d));
}


static vm_stack initialize_stack() {
    vm_stack stack;
    stack.at = malloc(STACK_SIZE * sizeof(value));
    stack.head = 0;
    stack.size = 0;

    return stack;
}

static void free_stack(vm_stack *s) {
    free(s->at);
    s->at = NULL;
}

virtual_machine initialize_vm() {
    virtual_machine vm;
    vm.stack = initialize_stack();
    vm.ip = 0;
    vm.state = 0;
    return vm;
}

uint8_t next(uint8_t *ip) {
    return *(++ip);
}

unsigned int execute(virtual_machine *vm, bytecode_array *bytecode) {
    if (vm == NULL || bytecode == NULL || bytecode->array == NULL) {
        return 1;
    }

    uint8_t *ip = bytecode->array;
    unsigned int size = bytecode->elements;

    for (int i = 0; i < size; i++) {
        switch (bytecode->array[i]) {
        case OP_RETURN:
            goto end;
        case OP_CONSTANT:
            push(&vm->stack, bytecode->constants.array[bytecode->array[i + 1]]);
            i++;
            break;
        case OP_ADD:
            op_add(&vm->stack);
            break;
        case OP_SUB:
            op_sub(&vm->stack);
            break;
        case OP_MULT:
            op_mult(&vm->stack);
            break;
        case OP_DIV:
            op_div(&vm->stack);
            break;
        default:
            printf("unknown instruction\n");
            goto end;
        }
    }
end:
    return 0;
}

void free_vm(virtual_machine *vm) {
    free_stack(&vm->stack);
}

#ifdef DEBUG_STACK
void print_stack(virtual_machine *vm) {
    unsigned int size = vm->stack.size;

    fputs("--- Contents of stack ---\n", stdout);
    for (int i = 0; i < size; i++) {
        printf("%d: %lf\n", i, vm->stack.at[i].as.d);
    }
}
#endif /* DEBUG_STACK */