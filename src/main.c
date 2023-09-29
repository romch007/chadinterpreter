#define CVECTOR_LOGARITHMIC_GROWTH

#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
    cvector_vector_type(token_t) tokens = tokenize("let result = 1 + 1; const pi = 314;");
    print_tokens(tokens);
    parser_t parser = {
            .token_index = 0,
            .tokens = tokens,
            .token_count = cvector_size(tokens),
    };
    statement_t* root = parse_statement(&parser);
    free_statement(root);
}
