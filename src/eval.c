#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
#include "getopt_impl.h"
#endif


#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include "errors.h"

#define STBDS_REALLOC(context,ptr,size) xrealloc(ptr, size)
#define STBDS_FREE(context,ptr)         free(ptr)
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "stb_extra.h"

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
    struct token* tokens = tokenize(content);
    free(content);
    // print_tokens(tokens);

    // Parsing
    struct parser parser;
    init_parser(&parser, tokens);
    struct statement* root = parse_block(&parser);

    FOR_EACH(struct token, token, tokens) {
        if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STR_LITERAL) {
            free(token->value.str);
        }
    }
    arrfree(tokens);

    if (should_print_ast) {
        fprintf(stderr, "--- AST dump ---\n");
        dump_statement(root, 0);
        fprintf(stderr, "----------------\n");
    }

    // Runtime
    struct context context;
    init_context(&context);

    push_stack_frame(&context);
    execute_statement(&context, root);
    pop_stack_frame(&context);

    destroy_statement(root);
    destroy_context(&context);

    return 0;
}
