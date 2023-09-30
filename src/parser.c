#include "parser.h"
#include <stdio.h>

static void consume(parser_t* parser, size_t count) {
    parser->token_index += count;
}

static token_t* peek(parser_t* parser, size_t advance) {
    return &parser->tokens[parser->token_index + advance];
}

static token_t* advance(parser_t* parser) {
    token_t* token = peek(parser, 0);
    parser->token_index++;
    return token;
}

static token_t* expect(parser_t* parser, token_t* token, token_type_t type) {
    if (token == NULL)
        token = peek(parser, 0);
    if (token->type != type) {
        printf("ERROR: invalid token type, expected %s but got %s\n", token_type_to_string(type), token_type_to_string(token->type));
        abort();
    }
    return token;
}

statement_t* parse(parser_t* parser) {
    statement_t* root = make_block_statement();

    for (;;) {
        token_t* token = peek(parser, 0);
        if (token->type == TOKEN_EOS) break;

        statement_t* statement = NULL;

        switch (token->type) {
            case TOKEN_LET:
            case TOKEN_CONST:
                statement = parse_variable_declaration(parser);
                break;
            default:
                printf("ERROR: invalid first token %s", token_type_to_string(token->type));
                abort();
        }

        expect(parser, advance(parser), TOKEN_SEMICOLON);

        cvector_push_back(root->op.block.statements, statement);
    }

    return root;
}

statement_t* parse_variable_declaration(parser_t* parser) {
    // let/const
    token_t* decl_op = advance(parser);
    bool constant = decl_op->type == TOKEN_CONST;

    // variable name
    token_t* ident_variable_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
    char* variable_name = ident_variable_name->value.str;

    // type info
    expect(parser, advance(parser), TOKEN_COLON);
    token_t* ident_type_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
    char* type_name = ident_type_name->value.str;

    // equal sign
    expect(parser, advance(parser), TOKEN_EQUAL);

    // variable value
    expr_t* value = parse_expression(parser);

    return make_variable_declaration(constant, variable_name, type_name, value);
}

expr_t* parse_expression(parser_t* parser) {
    expr_t* expr = parse_term(parser);

    for (;;) {
        token_t* token = peek(parser, 0);
        binary_opt_type_t op_type;

        bool invalid = false;
        switch (token->type) {
            case TOKEN_PLUS:
                op_type = BINARY_OP_ADD;
                break;
            case TOKEN_MINUS:
                op_type = BINARY_OP_SUB;
                break;
            default:
                invalid = true;
        }

        if (invalid) break;

        consume(parser, 1);
        expr = make_binary_op(op_type, expr, parse_term(parser));
    }

    return expr;
}

expr_t* parse_term(parser_t* parser) {
    expr_t* expr = parse_factor(parser);

    for (;;) {
        token_t* token = peek(parser, 0);
        binary_opt_type_t op_type;

        bool invalid = false;
        switch (token->type) {
            case TOKEN_MUL:
                op_type = BINARY_OP_MUL;
                break;
            case TOKEN_DIV:
                op_type = BINARY_OP_DIV;
                break;
            default:
                invalid = true;
        }

        if (invalid) break;

        consume(parser, 1);
        expr = make_binary_op(op_type, expr, parse_factor(parser));
    }

    return expr;
}

expr_t* parse_factor(parser_t* parser) {
    expr_t* expr;
    token_t* token = advance(parser);

    switch (token->type) {
        case TOKEN_INT_LITERAL:
            expr = make_integer_literal(token->value.integer);
            break;
        case TOKEN_STR_LITERAL:
            expr = make_string_literal(token->value.str);
            break;
        case TOKEN_IDENTIFIER:
            expr = make_variable_use(token->value.str);
            break;
        case TOKEN_OPEN_PAREN:
            expr = parse_expression(parser);
            expect(parser, advance(parser), TOKEN_CLOSE_PAREN);
            break;
        case TOKEN_PLUS:
            expr = parse_expression(parser);
            break;
        case TOKEN_MINUS:
            expr = make_unary_op(UNARY_OP_NEG, parse_term(parser));
            break;
        default:
            printf("ERROR: unexpected token %s", token_type_to_string(token->type));
            abort();
    }

    return expr;
}
