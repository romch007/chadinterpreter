#ifndef CHAD_INTERPRETER_PARSER_H
#define CHAD_INTERPRETER_PARSER_H

#include "ast.h"
#include "lexer.h"

struct parser {
    cvector_vector_type(struct token) tokens;
    size_t token_count;
    size_t token_index;
};

struct parser* create_parser(cvector_vector_type(struct token) tokens);

struct statement* parse_statement(struct parser* parser);
struct statement* parse_block(struct parser* parser);
struct statement* parse_if_condition(struct parser* parser);
struct statement* parse_variable_declaration(struct parser* parser);
struct statement* parse_function_declaration(struct parser* parser);
struct statement* parse_variable_assignment(struct parser* parser);
struct statement* parse_while_loop(struct parser* parser);
struct statement* parse_for_loop(struct parser* parser);
struct statement* parse_return_statement(struct parser* parser);

struct expr* parse_expression(struct parser* parser);
struct expr* parse_term(struct parser* parser);
struct expr* parse_factor(struct parser* parser);
struct expr* parse_function_call(struct parser* parser);

#endif
