#include "hashmap.h"
#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
#include "getopt_impl.h"
#endif


static char* read_file_content(char* filename) {
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        printf("ERROR: cannot open file: %s\n", strerror(errno));
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

void print_usage() {
    printf("Usage: eval [file]\n");
    printf("  -h: print help\n");
    printf("  -v: print version\n");
    printf("  -a: dump AST\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("ERROR: no file specified\n");
        print_usage();
        return 1;
    }

    bool should_print_ast = false;

    int opt;

    while ((opt = getopt(argc, argv, ":hva")) != -1) {
        switch (opt) {
            case 'a':
                should_print_ast = true;
                break;
            case 'h':
                print_usage();
                return 0;
            case 'v':
                printf("%s %s\n", APP_NAME, APP_VERSION);
                return 0;
            case '?':
                printf("ERROR: unknown option '%c'\n", optopt);
                print_usage();
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

    push_stack_frame(context);
    execute_statement(context, root);
    pop_stack_frame(context);

    destroy_statement(root);
    destroy_context(context);
    return 0;
}
