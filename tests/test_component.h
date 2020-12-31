#ifndef _TEST_COMPONENT_H_
#define _TEST_COMPONENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

#include "common.h"
#include "bytecode.h"
#include "tokenize.h"
#include "parser.h"
#include "vm.h"
#include "table.h"

int test_motmot_arithmetic() {
    INIT_TEST();

    BEGIN_TEST_CASE("2 + 2 compiles to expected bytecode");
    VirtualMachine vm = initialize_vm();
    char *code = "2 + 2";
    TokenArray *tokens = tokenize(code);
    BytecodeArray *chunk = parse(&vm, tokens);

    if (chunk->array[0] != OP_CONSTANT
            || chunk->array[1] != 0
            || chunk->array[2] != OP_CONSTANT
            || chunk->array[3] != 1
            || chunk->array[4] != OP_ADD) {
        TEST_FAIL();
    }

    free_array(tokens);
    free_bytecode_dynarray(chunk);
    free_vm(&vm);
    END_TEST_CASE();

    BEGIN_TEST_CASE("2 + 2 evaluates to 4");
    VirtualMachine vm = initialize_vm();
    char *code = "2 + 2";
    TokenArray *tokens = tokenize(code);
    BytecodeArray *chunk = parse(&vm, tokens);

    evaluate(&vm, chunk);

    if (pop(&vm.stack).as.real != 4.0) {
        TEST_FAIL();
    }

    free_array(tokens);
    free_bytecode_dynarray(chunk);
    free_vm(&vm);
    END_TEST_CASE();

    BEGIN_TEST_CASE("1 + 2 * 3 evaluates to 7");
    VirtualMachine vm = initialize_vm();
    char *code = "1 + 2 * 3";
    TokenArray *tokens = tokenize(code);
    BytecodeArray *chunk = parse(&vm, tokens);

    evaluate(&vm, chunk);

    if (pop(&vm.stack).as.real != 7.0) {
        TEST_FAIL();
    }

    free_array(tokens);
    free_bytecode_dynarray(chunk);
    free_vm(&vm);
    END_TEST_CASE();

    BEGIN_TEST_CASE("1 * 2 + 3 evaluates to 5");
    VirtualMachine vm = initialize_vm();
    char *code = "1 * 2 + 3";
    TokenArray *tokens = tokenize(code);
    BytecodeArray *chunk = parse(&vm, tokens);

    evaluate(&vm, chunk);

    if (pop(&vm.stack).as.real != 5.0) {
        TEST_FAIL();
    }

    free_array(tokens);
    free_bytecode_dynarray(chunk);
    free_vm(&vm);
    END_TEST_CASE();
    END_TEST();
}

#endif /* _TEST_COMPONENT_H_ */
