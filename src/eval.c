#include "hashmap.h"
#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char* read_file_content(char* filename) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("ERROR: file not found\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET); /* same as rewind(f); */

    char* string = xmalloc(fsize + 1);
    fread(string, fsize, 1, file);
    fclose(file);

    string[fsize] = 0;

    return string;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("ERROR: no file specified\n");
        return 1;
    }

    bool should_print_ast = false;

    int opt;

    while ((opt = getopt(argc, argv, ":ah")) != -1) {
        switch (opt) {
            case 'a':
                should_print_ast = true;
                break;
            case 'h':
                printf("Usage: eval [file]\n");
                printf("  -h: print help\n");
                printf("  -a: dump AST\n");
                return 0;
            case '?':
                printf("ERROR: unknown option %c\n", optopt);
                return 1;
            default:
                break;
        }
    }

    char* content = read_file_content(argv[optind]);

    // Lexing
    cvector_vector_type(token_t) tokens = tokenize(content);
    free(content);
    // print_tokens(tokens);

    // Parsing
    parser_t parser = {
            .token_index = 0,
            .tokens = tokens,
            .token_count = cvector_size(tokens),
    };
    statement_t* root = parse_block(&parser);

    cvector_set_elem_destructor(tokens, vector_token_deleter);
    cvector_free(tokens);

    if (should_print_ast) {
        printf("--- AST dump ---\n");
        dump_statement(root, 0);
        printf("----------------\n");
    }

    // Runtime
    context_t* context = create_context();
    execute_statement(context, root);

    destroy_statement(root);
    destroy_context(context);
    return 0;
}
