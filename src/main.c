#define CVECTOR_LOGARITHMIC_GROWTH

#include "hashmap.h"
#include "interpreter.h"
#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include <stdio.h>

static char* read_file_content(char* filename) {
    FILE* file = fopen(filename, "r");
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
    hashmap_set_allocator(&xmalloc, &free);

    if (argc < 2) {
        printf("ERROR: no file specified\n");
        return 1;
    }

    char* content = read_file_content(argv[1]);

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

    // Runtime
    context_t* context = create_context();
    execute_statement(context, root);

    dump_context(context);

    destroy_statement(root);
    destroy_context(context);
    return 0;
}
