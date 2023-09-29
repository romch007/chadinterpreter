#ifndef PNS_INTERPRETER_PARSER_H
#define PNS_INTERPRETER_PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
    cvector_vector_type(token_t) tokens;
    size_t token_count;
    size_t token_index;
} parser_t;

statement_t* parse_statement(parser_t* parser);
statement_t* parse_variable_declaration(parser_t* parser);

#endif