#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "table.h"
#include "function.h"
#include "error.h"
#include "lexer.h"


typedef unsigned char bool;

/**
 * 
 * 
 */


/* ---- static functions ---- */
static inline bool is_whitespace(const char c);
static inline bool is_num(const char c);
static inline bool is_operator(const char c);
static inline bool is_alpha(const char c);

static TokenArray *new_token_array();
static void resize_token_array(TokenArray *array, size_t new_size);
static void push_token(TokenArray *array, Token token);
static Token lex_number(char *code, unsigned int *i);
static TokenType get_keyword(char *code);
static Token lex_sqrt(char *code, unsigned int *i);
static Token lex_function(TokenArray *tokens, FunctionTable *functions, char *code, unsigned int *i);
static Token lex_identifier(TokenArray *tokens, FunctionTable *functions, char *code, unsigned int *i);

/*
 * Takes a string and returns a list of tokens to be parsed
 */
TokenArray *tokenize(char *code, FunctionTable *functions) {
    TokenArray *tokens = new_token_array();
    Token current;

    for (unsigned int i = 0;; i++) {
        current.string = NULL;
        char c = code[i];

        if (is_whitespace(c)) {
            continue;
        }

        switch (c) {
            /* end of string */
            case '\0': current.type = T_END; break;
            /* symbol */
            case '(':  current.type = T_LPAREN; break;
            case ')':  current.type = T_RPAREN; break;
            case '=':  if (code[i + 1] == '=') {
                    current.type = T_DBLEQL; break;
                } else {
                    current.type = T_EQUALS; break;
                }
            case '+':  current.type = T_PLUS; break;
            case '-':  current.type = T_MINUS; break;
            case '*':  current.type = T_STAR; break;
            case '/':  current.type = T_SLASH; break;
            case '%':  current.type = T_MOD; break;
            /* number */
            case '0'...'9': current = lex_number(code + i, &i); tokens->num_constants++; break;
            /* keyword or identifier */
            case 'A'...'Z':
            case 'a'...'z': {
                TokenType keyword = get_keyword(code + i);
                if (keyword == T_FUNC) { current = lex_function(tokens, functions, code + i, &i); break; }
                else if (keyword == T_SQRT) { current = lex_sqrt(code + i, &i); break; }
                else { current = lex_identifier(tokens, functions, code + i, &i); break; }
            }
            default: current.type = T_ERROR; break;
        }

        push_token(tokens, current);
        if (current.type == T_ERROR || current.type == T_END) {
            return tokens;
        }
    }
}

int tokenizer_had_error(TokenArray *array) {
    return array->tokens[array->elements - 1].type == T_ERROR;
}

/*
 * Free the token array returned by tokenize()
 */
void free_token_array(TokenArray *array) {
    // for (int i = 0; i < array->elements; i++) {
    //     if (array->tokens[i].type == T_FUNC || array->tokens[i].type == T_VAR) {
    //         free(array->tokens[i].string);
    //     }
    // }
    free(array->tokens);
    free(array->var_name);
    free(array);
    array = NULL;
}


#ifdef DEBUG
/*
 * Print out all of the tokens by type from a token array for debug purposes
 */
void print_tokens(TokenArray *tokens) {
    for (int i = 0; i < tokens->elements; i++) {
        switch (tokens->tokens[i].type) {
            case T_ERROR:   printf("(ERROR)"); break;
            case T_END:     printf("(END)"); break;
            case T_NUMBER:  printf("(NUMBER, %f)", tokens->tokens[i].value); break;
            case T_VAR:     printf("(VAR, %s)", tokens->tokens[i].string); break;
            case T_EQUALS:  printf("(EQUALS)"); break;
            case T_DBLEQL:  printf("(DBLEQL)"); break;
            case T_LPAREN:  printf("(LPAREN)"); break;
            case T_RPAREN:  printf("(RPAREN)"); break;
            case T_FUNC:    printf("(FUNC, %s)", tokens->tokens[i].string); break;
            case T_FUNCALL: printf("(FUNCALL, %s)", tokens->tokens[i].string); break;
            case T_PLUS:    printf("(PLUS)"); break;
            case T_MINUS:   printf("(MINUS)"); break;
            case T_STAR:    printf("(STAR)"); break;
            case T_SLASH:   printf("(SLASH)"); break;
            case T_SQRT:    printf("(SQRT)"); break;
        }
        printf(" ");
    }
    printf("\n");
}
#endif

/* ---- static functions ---- */
static inline bool is_whitespace(const char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

static inline bool is_num(const char c) {
    return c >= '0' && c <= '9';
}

static inline bool is_operator(const char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

static inline bool is_alpha(const char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static TokenArray *new_token_array() {
    TokenArray *array = malloc(sizeof *array);
    array->capacity = 8;
    array->elements = 0;
    array->num_constants = 0;
    array->in_function = 0;
    array->var_name = NULL;
    array->tokens = malloc(sizeof *array->tokens * array->capacity);
    return array;
}

static void resize_token_array(TokenArray *array, size_t new_size) {
    Token *old = array->tokens;
    array->tokens = malloc((sizeof *array->tokens) * new_size);
    array->capacity = new_size;

    for (unsigned int i = 0; i < array->elements; i++) {
        array->tokens[i] = old[i];
    }

    free(old);
}

static void push_token(TokenArray *array, Token token) {
    if (array->elements >= array->capacity) {
        resize_token_array(array, array->capacity * 2);
    }

    array->tokens[array->elements++] = token;
}

static Token lex_number(char *code, unsigned int *i) {
    char *begin = code;
    char *end = begin;
    bool seen_dot = 0;
    Token number;
    number.type = T_NUMBER;

    for (;;) {
        if (is_num(*end)) {
            end++;
        } else if (*end == '.') {
            if (seen_dot) {
                return (Token) { T_ERROR, { 0.0f } };
            }
            seen_dot = 1;
            end++;
        } else {
            *i += (end - begin) - 1;
            number.value = strtof(begin, &end);
            return number;
        }
    }
}

static TokenType get_keyword(char *code) {
    if (*code == 'f' && (strncmp(code, "function", 8) == 0)) { return T_FUNC; }
    if (*code == 's' && (strncmp(code, "sqrt", 4) == 0)) { return T_SQRT; }
    return T_ERROR;
}

static Token lex_sqrt(char *code, unsigned int *i) {
    *i += 3;
    return (Token) { T_SQRT, { 0.0f } };
}

static Token lex_function(TokenArray *tokens, FunctionTable *functions, char *code, unsigned int *i) {
    Token func;
    func.type = T_FUNC;
    func.string = NULL;
    char *begin = code + 8; /* skip past 'function', which has already been matched */

    for (;; begin++) {
        if (!is_whitespace(*begin)) {
            break;
        }

        (*i)++;
    }

    char *end = begin;
    while (is_alpha(*end)) {
        end++;
    }

    // check if function aliases a previously declared function or a keyword
    tokens->in_function = 1;
    *i += end - code - 2;

    if (end - begin == 0) {
        report_error("TokenError", "Function name empty.");
        return (Token) { T_ERROR };
    }

    func.string = strndup(begin, end - begin);

    if (strncmp(func.string, "sqrt", 4) == 0 || get_key_by_string_n(functions->table, func.string, (size_t) (end - begin)) != NULL) {
        report_error("TokenError", "Function name '%s' aliases another function or keyword.", func.string);
        free(func.string);
        return (Token) { T_ERROR };
    }
    return func;
}

static Token lex_identifier(TokenArray *tokens, FunctionTable *functions, char *code, unsigned int *i) {
    Token var;
    var.type = T_VAR;

    char *end = code;
    while (is_alpha(*end)) {
        end++;
    }

    if (tokens->in_function) {
        /* make sure var_name doesn't alias a function */
        if (tokens->var_name == NULL) {
            var.string = strndup(code, end - code);
            tokens->var_name = var.string;
            if (get_key_by_string_n(functions->table, code, (size_t) (end - code)) != NULL) {
                report_error("TokenError", "Parameter name '%s' aliases another function or keyword.", var.string);
                free(var.string);
                return (Token) { T_ERROR };
            }
            return var;
        } else if (strncmp(tokens->var_name, code, (size_t) (end - code)) == 0) {
            var.string = tokens->var_name;
            return var;
        }
    }

    char *func_name = get_key_by_string_n(functions->table, code, (size_t) (end - code));
    var.type = T_FUNCALL;
    var.string = func_name;
    // if (func_name == NULL) {
    //     char *name = strndup(code, end - code);
    //     report_error("TokenError", "Undefined function '%s'.", name);
    //     free(name);
    //     return (Token) { T_ERROR };
    // } else {
    //     var.type = T_FUNCALL;
    //     var.string = func_name;
    // }

    // if just lexxed function, treat this variable as the main variable
    // check if it's also a function name to avoid aliasing
    // every other identifier which is a variable will point to the main
    // variable's string for hash table indexing
    // so tokens needs to store, like, if it's in a function & a pointer
    // to the variable string

    // if it's a function call then get the key from the functiontable
    // and store it in the token (FUNCALL, key)
    // check function table if string exists
    // elif in function check if string == variable

    *i += end - code - 1;
    return var;
}