#include <stdio.h>

#include "parser.h"
#include "errors.h"
#include "mem.h"
#include "stb_ds.h"
#include "stb_extra.h"

static void consume(struct parser* parser, size_t count) {
    parser->token_index += count;
}

static struct token* peek(struct parser* parser, size_t advance) {
    return &parser->tokens[parser->token_index + advance];
}

static struct token* advance(struct parser* parser) {
    struct token* token = peek(parser, 0);
    parser->token_index++;
    return token;
}

static struct token* expect(struct parser* parser, struct token* token, enum token_type type) {
    if (token == NULL)
        token = peek(parser, 0);
    if (token->type != type) {
        panic("ERROR: invalid token type line %d, expected %s but got %s\n", token->line_nb, token_type_to_string(type), token_type_to_string(token->type));
    }
    return token;
}

struct parser* create_parser(struct token* tokens) {
  struct parser* parser = xmalloc(sizeof(struct parser));
  parser->token_index = 0;
  parser->tokens = tokens;
  parser->token_count = arrlen(tokens);
  return parser;
}

struct statement* parse_statement(struct parser* parser) {
    struct token* token = peek(parser, 0);

    if (token->type == TOKEN_EOS || token->type == TOKEN_CLOSE_BRACE) return NULL;

    struct statement* statement;

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
        case TOKEN_FN:
            statement = parse_function_declaration(parser);
            break;
        case TOKEN_IF:
            statement = parse_if_condition(parser);
            break;
        case TOKEN_WHILE:
            statement = parse_while_loop(parser);
            break;
        case TOKEN_FOR:
            statement = parse_for_loop(parser);
            break;
        case TOKEN_BREAK:
            consume(parser, 1);
            statement = make_break_statement();
            expect(parser, advance(parser), TOKEN_SEMICOLON);
            break;
        case TOKEN_CONTINUE:
            consume(parser, 1);
            statement = make_continue_statement();
            expect(parser, advance(parser), TOKEN_SEMICOLON);
            break;
        case TOKEN_RETURN:
            statement = parse_return_statement(parser);
            break;
        case TOKEN_OPEN_BRACE:
            expect(parser, advance(parser), TOKEN_OPEN_BRACE);
            statement = parse_block(parser);
            expect(parser, advance(parser), TOKEN_CLOSE_BRACE);
            break;
        default:
            panic("ERROR: invalid token %s", token_type_to_string(token->type));
    }

    return statement;
}

struct statement* parse_block(struct parser* parser) {
    struct statement* root = make_block_statement();

    for (;;) {
        struct statement* statement = parse_statement(parser);

        if (statement == NULL) break;

        arrpush(root->op.block.statements, statement);
    }

    return root;
}

struct statement* parse_if_condition(struct parser* parser) {
    expect(parser, advance(parser), TOKEN_IF);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);
    struct expr* condition = parse_expression(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    expect(parser, advance(parser), TOKEN_OPEN_BRACE);
    struct statement* body = parse_block(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_BRACE);

    struct statement* if_condition = make_if_condition_statement(condition, body);

    if (peek(parser, 0)->type == TOKEN_ELSE) {
        consume(parser, 1);

        if (peek(parser, 0)->type == TOKEN_IF) {
            if_condition->op.if_condition.body_else = parse_if_condition(parser);
        } else {
            expect(parser, advance(parser), TOKEN_OPEN_BRACE);
            if_condition->op.if_condition.body_else = parse_block(parser);
            expect(parser, advance(parser), TOKEN_CLOSE_BRACE);
        }
    }

    return if_condition;
}

struct statement* parse_variable_declaration(struct parser* parser) {
    struct token* decl_op = advance(parser);
    bool constant = decl_op->type == TOKEN_CONST;

    struct token* ident_variable_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
    char* variable_name = ident_variable_name->value.str;

    struct statement* variable_declaration = make_variable_declaration(constant, variable_name);

    if (peek(parser, 0)->type == TOKEN_EQUAL) {
        consume(parser, 1);
        variable_declaration->op.variable_declaration.value = parse_expression(parser);
    }

    expect(parser, advance(parser), TOKEN_SEMICOLON);

    return variable_declaration;
}

struct statement* parse_function_declaration(struct parser* parser) {
    expect(parser, advance(parser), TOKEN_FN);
    struct token* ident_fn_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);

    struct statement* fn_decl = make_function_declaration(ident_fn_name->value.str);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);

    if (peek(parser, 0)->type == TOKEN_IDENTIFIER) {
        for (;;) {
            struct token* ident_arg_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
            arrpush(fn_decl->op.function_declaration.arguments, xstrdup(ident_arg_name->value.str));

            if (peek(parser, 0)->type == TOKEN_COMMA) {
                consume(parser, 1);
            } else {
                break;
            }
        }
    }

    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    expect(parser, advance(parser), TOKEN_OPEN_BRACE);
    fn_decl->op.function_declaration.body = parse_block(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_BRACE);

    return fn_decl;
}

struct statement* parse_variable_assignment(struct parser* parser) {
    struct token* ident_variable_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);
    char* variable_name = ident_variable_name->value.str;
    struct expr* value = NULL;

    if (peek(parser, 0)->type == TOKEN_PLUS_EQUAL) {
        consume(parser, 1);

        value = make_binary_op(BINARY_OP_ADD, make_variable_use(variable_name), parse_expression(parser));
    } else if (peek(parser, 0)->type == TOKEN_MINUS_EQUAL) {
        consume(parser, 1);

        value = make_binary_op(BINARY_OP_SUB, make_variable_use(variable_name), parse_expression(parser));
    } else if (peek(parser, 0)->type == TOKEN_MUL_EQUAL) {
        consume(parser, 1);

        value = make_binary_op(BINARY_OP_MUL, make_variable_use(variable_name), parse_expression(parser));
    } else if (peek(parser, 0)->type == TOKEN_DIV_EQUAL) {
        consume(parser, 1);

        value = make_binary_op(BINARY_OP_DIV, make_variable_use(variable_name), parse_expression(parser));
    } else {
        expect(parser, advance(parser), TOKEN_EQUAL);
        value = parse_expression(parser);
    }

    expect(parser, advance(parser), TOKEN_SEMICOLON);

    return make_variable_assignment(variable_name, value);
}

struct statement* parse_while_loop(struct parser* parser) {
    expect(parser, advance(parser), TOKEN_WHILE);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);
    struct expr* condition = parse_expression(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    expect(parser, advance(parser), TOKEN_OPEN_BRACE);
    struct statement* body = parse_block(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_BRACE);

    return make_while_loop(condition, body);
}

struct statement* parse_for_loop(struct parser* parser) {
    expect(parser, advance(parser), TOKEN_FOR);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);

    struct statement* initializer = parse_statement(parser);
    struct expr* condition = parse_expression(parser);
    expect(parser, advance(parser), TOKEN_SEMICOLON);
    struct statement* increment = parse_statement(parser);

    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    expect(parser, advance(parser), TOKEN_OPEN_BRACE);
    struct statement* body = parse_block(parser);
    expect(parser, advance(parser), TOKEN_CLOSE_BRACE);

    return make_for_loop(initializer, condition, increment, body);
}

struct statement* parse_return_statement(struct parser* parser) {
    expect(parser, advance(parser), TOKEN_RETURN);

    struct statement* return_stmt = make_return_statement();

    if (peek(parser, 0)->type != TOKEN_SEMICOLON) {
        return_stmt->op.return_statement.value = parse_expression(parser);
    }

    expect(parser, advance(parser), TOKEN_SEMICOLON);

    return return_stmt;
}

struct expr* parse_expression(struct parser* parser) {
    struct expr* expr = parse_term(parser);

    for (;;) {
        struct token* token = peek(parser, 0);
        enum binary_op_type op_type;

        bool invalid = false;
        switch (token->type) {
            case TOKEN_PLUS:
                op_type = BINARY_OP_ADD;
                break;
            case TOKEN_MINUS:
                op_type = BINARY_OP_SUB;
                break;
            case TOKEN_OR:
                op_type = BINARY_OP_OR;
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

struct expr* parse_term(struct parser* parser) {
    struct expr* expr = parse_factor(parser);

    for (;;) {
        struct token* token = peek(parser, 0);
        enum binary_op_type op_type;

        bool invalid = false;
        switch (token->type) {
            case TOKEN_MUL:
                op_type = BINARY_OP_MUL;
                break;
            case TOKEN_DIV:
                op_type = BINARY_OP_DIV;
                break;
            case TOKEN_MODULO:
                op_type = BINARY_OP_MODULO;
                break;
            case TOKEN_AND:
                op_type = BINARY_OP_AND;
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

struct expr* parse_factor(struct parser* parser) {
    struct expr* expr;
    struct token* token = peek(parser, 0);

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
        case TOKEN_NULL:
            consume(parser, 1);
            expr = make_null();
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
            panic("ERROR: unexpected token %s", token_type_to_string(token->type));
    }

    return expr;
}

struct expr* parse_function_call(struct parser* parser) {
    struct token* ident_fn_name = expect(parser, advance(parser), TOKEN_IDENTIFIER);

    expect(parser, advance(parser), TOKEN_OPEN_PAREN);
    struct expr* function_call = make_function_call(ident_fn_name->value.str);

    while (peek(parser, 0)->type != TOKEN_CLOSE_PAREN) {
        struct expr* argument = parse_expression(parser);
        arrpush(function_call->op.function_call.arguments, argument);

        if (peek(parser, 0)->type != TOKEN_COMMA)
            break;
        else
            consume(parser, 1);
    }

    expect(parser, advance(parser), TOKEN_CLOSE_PAREN);

    return function_call;
}
