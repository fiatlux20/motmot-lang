#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "table.h"
#include "function.h"
#include "error.h"

void run(char *code, FunctionTable *functions) {
    TokenArray *tokens = tokenize(code, functions);

    // print_tokens(tokens);
    if (tokenizer_had_error(tokens)) {
        free_token_array(tokens);
        return;
    }

    // if (tokens->func_name != NULL) {
    //     move_key(functions->keys, tokens->func_name);
    // }

    ParserState *parser = parse(tokens, functions);

    if (parser->error) {
        free_token_array(tokens);
        free_parser(parser);
        return;
    }

    if (!parser->parse_function) {
        float (*compiled)(float*) = compile_expression(functions, parser->code);
        printf("%f\n", compiled(parser->constants));
    } else {
        compile_function(functions, parser->code);
    }

    #ifdef DEBUG
    disassemble(parser->code);
    #endif
    // printf("----\n");
    // dump_memory_range(functions, 0x00, 0x100);

    free_token_array(tokens);
    free_parser(parser);
}

void run_interactive() {
    FunctionTable *functions = create_function_table();
    init_error_handler(functions);

    char buf[1024];
    for (;;) {
        printf(">>> ");

        if (fgets(buf, 1024, stdin) == NULL) {
            fputc('\n', stdout);
            break;
        }

        run(buf, functions);
    }

    free_function_table(functions);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        run_interactive();
    } else if (argc == 2) {

    } else {

    }
    return 0;
}
