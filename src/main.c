#include <stdio.h>
#include <stdlib.h>

#include "tokenize.h"

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

int run(char *source) {
    linked_token_list *tokens = tokenize(source);

    print_tokens(tokens);
    free_tokens(tokens);

    return 0;
}

#define IN_BUF_SZ 1024
int run_interactive() {
    fputs("Running in interpreter mode.\n", stdout);
    char input[IN_BUF_SZ];

    for (;;) {
        printf(">>> ");
        if (fgets(input, IN_BUF_SZ, stdin) == NULL) {
            fputs("\n", stdout);
            break;
        }

        run(input);
    }

    return 0;
}

int run_file(char *filename) {
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

    run(source);

    fclose(fp);
    free(source);
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
