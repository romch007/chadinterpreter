// clang-format off
#include <stdio.h>
#include <readline/history.h>
#include <readline/readline.h>
// clang-format on

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

int main() {
    context_t* context = create_context();
    for (;;) {
        char* prompt = readline("> ");

        if (prompt == NULL) exit(EXIT_SUCCESS);

        add_history(prompt);

        cvector_vector_type(token_t) tokens = tokenize(prompt);
        parser_t parser = {
                .tokens = tokens,
                .token_index = 0,
                .token_count = cvector_size(tokens),
        };
        statement_t* root = parse_block(&parser);

        cvector_set_elem_destructor(tokens, vector_token_deleter);
        cvector_free(tokens);

        execute_statement(context, root);
        dump_context(context);
        destroy_statement(root);
    }
    destroy_context(context);
    return 0;
}
