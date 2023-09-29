#include "parser.h"
#include <stdio.h>

static void consume(parser_t* parser, size_t count) {
    parser->token_index += count;
}

static token_t* peek(parser_t* parser, size_t advance) {
    return &parser->tokens[parser->token_index + advance];
}

static token_t* advance(parser_t* parser) {
    token_t* token = peek(parser, 1);
    parser->token_index++;
    return token;
}

static token_t* expect(parser_t* parser, token_t* token, token_type_t type) {
    if (token == NULL)
        token = peek(parser, 1);
    if (token->type != type) {
        printf("ERROR: invalid token type\n");
        abort();
    }
    return token;
}

statement_t* parse_statement(parser_t* parser) {
    statement_t* root = make_block_statement();

    do {
        token_t* token = peek(parser, 1);
        
        switch (token->type) {
            case TOKEN_LET:
            case TOKEN_CONST:
                // parse_variable_declaraction
        }
    } while (advance(parser)->type == TOKEN_EOS);

    return root;
}
