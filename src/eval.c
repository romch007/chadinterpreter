#include "hashmap.h"
#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
#include "getopt_impl.h"
#endif


static char* read_file_content(char* filename) {
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        panic("ERROR: cannot open file: %s\n", strerror(errno));
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
        fprintf(stderr, "ERROR: no file specified\n");
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
                fprintf(stderr, "ERROR: unknown option '%c'\n", optopt);
                print_usage();
                return 1;
            default:
                break;
        }
    }

    char* content = read_file_content(argv[optind]);

    // Lexing
    cvector_vector_type(struct token) tokens = tokenize(content);
    free(content);
    // print_tokens(tokens);

    // Parsing
    struct parser* parser = create_parser(tokens);
    struct statement* root = parse_block(parser);

    cvector_set_elem_destructor(tokens, vector_token_deleter);
    cvector_free(tokens);

    if (should_print_ast) {
        fprintf(stderr, "--- AST dump ---\n");
        dump_statement(root, 0);
        fprintf(stderr, "----------------\n");
    }

    // Runtime
    struct context* context = create_context();

    push_stack_frame(context);
    execute_statement(context, root);
    pop_stack_frame(context);

    destroy_statement(root);
    destroy_context(context);
    free(parser);

    return 0;
}
