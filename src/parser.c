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
        exit(EXIT_FAILURE);
    }
    return token;
}

statement_t* parse_block(parser_t* parser) {
    statement_t* root = make_block_statement();

    for (;;) {
        token_t* token = peek(parser, 0);
        if (token->type == TOKEN_EOS || token->type == TOKEN_CLOSE_BRACE) break;

        statement_t* statement = NULL;

        switch (token->type) {
            case TOKEN_LET:
            case TOKEN_CONST:
                statement = parse_variable_declaration(parser);
                break;
            case TOKEN_IDENTIFIER:
                if (peek(parser, 1)->type == TOKEN_OPEN_PAREN) {
                    statement = make_naked_fn_call(parse_function_call(parser));
                    expect(parser, advance(parser), TOKEN_SEMICOLON);
                } else {
                    statement = parse_variable_assignment(parser);
                }
                break;
            case TOKEN_IF:
                statement = parse_if_condition(parser);
                break;
            case TOKEN_WHILE:
                statement = parse_while_loop(parser);
                break;
            case TOKEN_OPEN_BRACE:
                expect(parser, advance(parser), TOKEN_OPEN_BRACE);
                statement = parse_block(parser);
                expect(parser, advance(parser), TOKEN_CLOSE_BRACE);
                break;
            default:
                printf("ERROR: invalid first token %s", token_type_to_string(token->type));
                exit(EXIT_FAILURE);
        }

        cvector_push_back(root->op.block.statements, statement);
    }

    return root;
}

statement_t* parse_if_condition(parser_t* parser) {
    expect(parser, advance(parser), TOKEN_IF);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);
    expr_t* condition = parse_expression(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    expect(parser, advance(parser), TOKEN_OPEN_BRACE);
    statement_t* body = parse_block(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_BRACE);

    statement_t* if_condition = make_if_condition_statement(condition, body);

    if (peek(parser, 0)->type == TOKEN_ELSE) {
        consume(parser, 1);

        expect(parser, advance(parser), TOKEN_OPEN_BRACE);
        if_condition->op.if_condition.body_else = parse_block(parser);
        expect(parser, advance(parser), TOKEN_CLOSE_BRACE);
    }

    return if_condition;
}

statement_t* parse_variable_declaration(parser_t* parser) {
    // let/const
    token_t* decl_op = advance(parser);
    bool constant = decl_op->type == TOKEN_CONST;

    // variable name
    token_t* ident_variable_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
    char* variable_name = ident_variable_name->value.str;

    // equal sign
    expect(parser, advance(parser), TOKEN_EQUAL);

    // variable value
    expr_t* value = parse_expression(parser);

    expect(parser, advance(parser), TOKEN_SEMICOLON);

    return make_variable_declaration(constant, variable_name, value);
}

statement_t* parse_variable_assignment(parser_t* parser) {
    token_t* ident_variable_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
    char* variable_name = ident_variable_name->value.str;

    expect(parser, advance(parser), TOKEN_EQUAL);

    expr_t* value = parse_expression(parser);

    expect(parser, advance(parser), TOKEN_SEMICOLON);

    return make_variable_assignment(variable_name, value);
}

statement_t* parse_while_loop(parser_t* parser) {
    expect(parser, advance(parser), TOKEN_WHILE);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);
    expr_t* condition = parse_expression(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    expect(parser, advance(parser), TOKEN_OPEN_BRACE);
    statement_t* body = parse_block(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_BRACE);

    return make_while_loop(condition, body);
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
            case TOKEN_DOUBLE_EQUAL:
                op_type = BINARY_OP_EQUAL;
                break;
            case TOKEN_NOT_EQUAL:
                op_type = BINARY_OP_NOT_EQUAL;
                break;
            case TOKEN_GREATER:
                op_type = BINARY_OP_GREATER;
                break;
            case TOKEN_GREATER_EQUAL:
                op_type = BINARY_OP_GREATER_EQUAL;
                break;
            case TOKEN_LESS:
                op_type = BINARY_OP_LESS;
                break;
            case TOKEN_LESS_EQUAL:
                op_type = BINARY_OP_LESS_EQUAL;
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
            case TOKEN_AND:
                op_type = BINARY_OP_AND;
                break;
            case TOKEN_OR:
                op_type = BINARY_OP_OR;
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
    token_t* token = peek(parser, 0);

    switch (token->type) {
        case TOKEN_BOOL_LITERAL:
            consume(parser, 1);
            expr = make_bool_literal(token->value.boolean);
            break;
        case TOKEN_INT_LITERAL:
            consume(parser, 1);
            expr = make_integer_literal(token->value.integer);
            break;
        case TOKEN_FLOAT_LITERAL:
            consume(parser, 1);
            expr = make_float_literal(token->value.floating);
            break;
        case TOKEN_STR_LITERAL:
            consume(parser, 1);
            expr = make_string_literal(token->value.str);
            break;
        case TOKEN_IDENTIFIER:
            if (peek(parser, 1)->type == TOKEN_OPEN_PAREN) {
                expr = parse_function_call(parser);
            } else {
                consume(parser, 1);
                expr = make_variable_use(token->value.str);
            }
            break;
        case TOKEN_OPEN_PAREN:
            consume(parser, 1);
            expr = parse_expression(parser);
            expect(parser, advance(parser), TOKEN_CLOSE_PAREN);
            break;
        case TOKEN_PLUS:
            consume(parser, 1);
            expr = parse_expression(parser);
            break;
        case TOKEN_MINUS:
            consume(parser, 1);
            expr = make_unary_op(UNARY_OP_NEG, parse_term(parser));
            break;
        case TOKEN_NOT:
            consume(parser, 1);
            expr = make_unary_op(UNARY_OP_NOT, parse_term(parser));
            break;
        default:
            printf("ERROR: unexpected token %s", token_type_to_string(token->type));
            exit(EXIT_FAILURE);
    }

    return expr;
}

expr_t* parse_function_call(parser_t* parser) {
    token_t* ident_fn_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);
    expr_t* function_call = make_function_call(ident_fn_name->value.str);

    while (peek(parser, 0)->type != TOKEN_CLOSE_PAREN) {
        expr_t* argument = parse_expression(parser);
        cvector_push_back(function_call->op.function_call.arguments, argument);

        if (peek(parser, 0)->type != TOKEN_COMMA)
            break;
        else
            consume(parser, 1);
    }

    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    return function_call;
}
