#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vm.h"

static void push(vm_stack *s, Value val) {
    s->head &= s->size - 1;
    s->at[s->head++] = val;
#ifdef DEBUG_STACK
    if (val.type == VAL_TYPE_DOUBLE) {
        printf("pushing double '%lf'. stack head now at %d\n", val.as.real, s->head);
    } else if (val.type == VAL_TYPE_INTEGER) {
        printf("pushing int '%ld'. stack head now at %d\n", val.as.integer, s->head);
    } else if (val.type == VAL_TYPE_STRING) {
        printf("pushing string '%s'. stack head now at %d\n", val.as.string, s->head);
    }
#endif
}

static Value pop(vm_stack *s) {
    s->head &= s->size - 1;
    s->head--;

#ifdef DEBUG_STACK
    Value val = s->at[s->head];
    if (val.type == VAL_TYPE_DOUBLE) {
        printf("popping double '%lf'. stack head now at %d\n", val.as.real, s->head);
    } else if (val.type == VAL_TYPE_INTEGER) {
        printf("popping int '%ld'. stack head now at %d\n", val.as.integer, s->head);
    } else if (val.type == VAL_TYPE_STRING) {
        printf("popping string '%s'. stack head now at %d\n", val.as.string, s->head);
    }

    return val;
#else
    return s->at[s->head];
#endif
}

/* opcodes */
static void op_add(vm_stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    if (a.type == VAL_TYPE_INTEGER) {
        printf("value a is of type int\n");
    } else if (a.type == VAL_TYPE_DOUBLE) {
        printf("value a is of type double: %lf\n", a.as.real);
    }

    if (b.type == VAL_TYPE_INTEGER) {
        printf("value b is of type int\n");
    } else if (b.type == VAL_TYPE_DOUBLE) {
        printf("value b is of type double: %lf\n", b.as.real);
    }

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, double_value(a.as.real + b.as.real));
    } else if (a.type == VAL_TYPE_INTEGER && b.type == VAL_TYPE_INTEGER) {
        push(s, int_value(a.as.integer + b.as.integer));
    } else if (a.type == VAL_TYPE_STRING && b.type == VAL_TYPE_STRING) {
        push(s, add_strings(&a, &b));
    } else {
        // error
    }
}

static void op_sub(vm_stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, double_value(a.as.real - b.as.real));
    } else if (a.type == VAL_TYPE_INTEGER && b.type == VAL_TYPE_INTEGER) {
        push(s, int_value(a.as.integer - b.as.integer));
    } else {
        // error
    }
}

static void op_mult(vm_stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    push(s, int_value(a.as.integer * b.as.integer));
}

static void op_div(vm_stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    push(s, int_value(b.as.integer / a.as.integer));
}


static vm_stack initialize_stack() {
    vm_stack stack;
    stack.at = malloc((sizeof *stack.at) * STACK_SIZE);
    stack.head = 0;
    stack.size = STACK_SIZE;

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
    unsigned int top = vm->stack.head;
    Value *v = vm->stack.at;

    fputs("--- Contents of stack ---\n", stdout);
    for (int i = 0; i < top; i++) {
        printf("%02x: ", i);
        print_value(v++);
        printf("\n");
    }
}
#endif /* DEBUG_STACK */
