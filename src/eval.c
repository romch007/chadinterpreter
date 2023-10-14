#include "hashmap.h"
#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
char* optarg;
int optind = 1, opterr = 1, optopt, __optpos, optreset = 0;

#define optpos __optpos

static void __getopt_msg(const char* a, const char* b, const char* c, size_t l) {
    FILE* f = stderr;
#if !defined(WIN32) && !defined(_WIN32)
    flockfile(f);
#endif
    fputs(a, f);
    fwrite(b, strlen(b), 1, f);
    fwrite(c, 1, l, f);
    fputc('\n', f);
#if !defined(WIN32) && !defined(_WIN32)
    funlockfile(f);
#endif
}

static int getopt(int argc, char* const argv[], const char* optstring) {
    int i, c, d;
    int k, l;
    char* optchar;

    if (!optind || optreset) {
        optreset = 0;
        __optpos = 0;
        optind = 1;
    }

    if (optind >= argc || !argv[optind])
        return -1;

    if (argv[optind][0] != '-') {
        if (optstring[0] == '-') {
            optarg = argv[optind++];
            return 1;
        }
        return -1;
    }

    if (!argv[optind][1])
        return -1;

    if (argv[optind][1] == '-' && !argv[optind][2])
        return optind++, -1;

    if (!optpos) optpos++;
    c = argv[optind][optpos], k = 1;
    optchar = argv[optind] + optpos;
    optopt = c;
    optpos += k;

    if (!argv[optind][optpos]) {
        optind++;
        optpos = 0;
    }

    if (optstring[0] == '-' || optstring[0] == '+')
        optstring++;

    i = 0;
    d = 0;
    do {
        d = optstring[i], l = 1;
        if (l > 0) i += l;
        else
            i++;
    } while (l && d != c);

    if (d != c) {
        if (optstring[0] != ':' && opterr)
            __getopt_msg(argv[0], ": unrecognized option: ", optchar, k);
        return '?';
    }
    if (optstring[i] == ':') {
        if (optstring[i + 1] == ':') optarg = 0;
        else if (optind >= argc) {
            if (optstring[0] == ':') return ':';
            if (opterr) __getopt_msg(argv[0],
                                     ": option requires an argument: ",
                                     optchar, k);
            return '?';
        }
        if (optstring[i + 1] != ':' || optpos) {
            optarg = argv[optind++] + optpos;
            optpos = 0;
        }
    }
    return c;
}


#endif


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
