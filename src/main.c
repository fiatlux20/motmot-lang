#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "bytecode.h"
#include "tokenize.h"
#include "parser.h"
#include "vm.h"
#include "table.h"

unsigned long get_file_size(FILE *fp) {
    if (fp == NULL) {
        return 0L;
    }

    unsigned long orig_pos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    unsigned long file_size = ftell(fp);
    fseek(fp, orig_pos, SEEK_SET);
    return file_size;
}

int run(virtual_machine *vm, char *source) {
    TokenArray *tokens = tokenize(source);

    #ifdef DEBUG_TOKENS
    print_tokens(tokens);
    #endif

    bytecode_array bytecode = parse(vm, tokens);
    free_array(tokens);

    #ifdef DEBUG_COMPILER
    print_disassembly(&bytecode);
    print_constants(&bytecode);
    print_names(&bytecode);
    #endif

    execute(vm, &bytecode);

    #ifdef DEBUG_STACK
    print_stack(vm);
    #endif

    free_bytecode_dynarray(&bytecode);

    return 0;
}

int run_interactive() {
    virtual_machine vm = initialize_vm();
#ifdef MAJOR_VERS
    printf("Motmot v%d.%d ", MAJOR_VERS, MINOR_VERS);
#endif
    fputs("Running in interactive mode.\n", stdout);
    char input[INPUT_BUFFER_SIZE];

    for (;;) {
        printf(">>> ");
        if (fgets(input, INPUT_BUFFER_SIZE, stdin) == NULL) {
            fputs("\n", stdout);
            break;
        }

        run(&vm, input);
    }

    free_vm(&vm);
    return 0;
}

int run_file(char *filename) {
    virtual_machine vm;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    unsigned long file_size = get_file_size(fp);
    char *source = malloc(sizeof(char) * (file_size + 1));
    unsigned long bytes_read = fread(source, sizeof(char), file_size, fp);

    if (bytes_read != file_size) {
        fprintf(stderr, "problem reading file\n");
        return -1;
    }

    source[file_size] = '\0';

    vm = initialize_vm();
    run(&vm, source);

    fclose(fp);
    free(source);
    free_vm(&vm);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        /* interactive mode */
        int error = run_interactive();

        if (error != 0) {
            fprintf(stderr, "errors occurred.\n");
        }
    } else if (argc == 2) {
        /* execute file */
        int error = run_file(argv[1]);

        if (error != 0) {
            fprintf(stderr, "Error executing file: '%s'\n", argv[1]);
        }
    } else {
        fprintf(stderr, "Too many arguments\n");
    }

    return 0;
}
