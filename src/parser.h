#ifndef CHAD_INTERPRETER_PARSER_H
#define CHAD_INTERPRETER_PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
    cvector_vector_type(token_t) tokens;
    size_t token_count;
    size_t token_index;
} parser_t;

statement_t* parse_block(parser_t* parser);
statement_t* parse_if_condition(parser_t* parser);
statement_t* parse_variable_declaration(parser_t* parser);
statement_t* parse_function_declaration(parser_t* parser);
statement_t* parse_variable_assignment(parser_t* parser);
statement_t* parse_while_loop(parser_t* parser);
statement_t* parse_return_statement(parser_t* parser);

expr_t* parse_expression(parser_t* parser);
expr_t* parse_term(parser_t* parser);
expr_t* parse_factor(parser_t* parser);
expr_t* parse_function_call(parser_t* parser);

#endif
