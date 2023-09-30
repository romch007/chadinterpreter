#define CVECTOR_LOGARITHMIC_GROWTH

#include "debug.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>

static char* read_file_content(char* filename) {
    FILE* file = fopen(filename, "r");
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET); /* same as rewind(f); */

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, file);
    fclose(file);

    string[fsize] = 0;

    return string;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("ERROR: no file specified");
        return 1;
    }

    char* content = read_file_content(argv[1]);
    cvector_vector_type(token_t) tokens = tokenize(content);
    print_tokens(tokens);

    parser_t parser = {
            .token_index = 0,
            .tokens = tokens,
            .token_count = cvector_size(tokens),
    };
    statement_t* root = parse(&parser);
    cvector_free(tokens);

    free_statement(root);
}
