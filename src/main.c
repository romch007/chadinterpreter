#include "lexer.h"

int main(int argc, char** argv) {
    // ast_expr_t* exp = make_binary_op(ADD, make_integer_literal(4), make_string_literal("hey"));

    vc_vector* tokens = tokenize("(44 + 'abc?!') - 0");
    print_tokens(tokens);
    vc_vector_release(tokens);
}
