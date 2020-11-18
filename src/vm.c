#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vm.h"

static void push(vm_stack *s, value val) {
    if (s->head >= STACK_SIZE) {
        s->head = 0;
    }

    #ifdef DEBUG_VM
    printf("pushing %lf. stack head at %d\n", val.d, s->head);
    #endif

    s->size++;
    s->at[s->head++] = val;
}

static value pop(vm_stack *s) {
    if (s->head < 0) {
        s->head = STACK_SIZE - 1;
    }

    #ifdef DEBUG_VM
    value val = s->at[--s->head];
    printf("popping %lf. stack head at %d\n", val.d, s->head);

    s->size--;
    return val;
    #else
    s->size--;
    return s->at[--s->head];
    #endif
}

static void op_add(vm_stack *s) {
    value a = pop(s);
    value b = pop(s);

    push(s, create_number(a.d + b.d));
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
        default:
            printf("unknown instruction\n");
            goto end;
        }
    }

    // for(;;) {
    //     switch (*(ip++)) {
    //     case OP_RETURN:
    //         goto end;
    //     case OP_CONSTANT:
    //         push(&vm->stack, bytecode->constants.array[next(ip)]);
    //         break;
    //     case OP_ADD:
    //         op_add(&vm->stack);
    //         break;
    //     default:
    //         break;
    //     }
    // }

end:
    return 0;
}

void free_vm(virtual_machine *vm) {
    free_stack(&vm->stack);
}

#ifdef DEBUG_VM
void print_stack(virtual_machine *vm) {
    unsigned int size = vm->stack.size;

    fputs("--- Contents of stack ---\n", stdout);
    for (int i = 0; i < size; i++) {
        printf("%d: %lf\n", i, vm->stack.at[i].d);
    }
}
#endif /* DEBUG_VM */