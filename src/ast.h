#ifndef PNS_INTERPRETER_AST_H
#define PNS_INTERPRETER_AST_H

#include "cvector.h"
#include <stdbool.h>

typedef enum {
    BINARY_OPT,
    UNARY_OPT,
    INT_LITERAL,
    STRING_LITERAL,
    VARIABLE_USE
} expr_type_t;

typedef enum {
    BINARY_ADD,
    BINARY_SUB,
    BINARY_MUL,
    BINARY_DIV
} binary_opt_type_t;

typedef enum {
    UNARY_NEG
} unary_opt_type_t;

typedef struct expr {
    expr_type_t type;
    union {
        int integer_literal;
        char* string_literal;
        struct {
            binary_opt_type_t type;
            struct expr* lhs;
            struct expr* rhs;
        } binary;
        struct {
            unary_opt_type_t type;
            struct expr* arg;
        } unary;
        struct {
            char* name;
        } variable_use;
    } op;
} expr_t;

expr_t* make_binary_op(binary_opt_type_t type, expr_t* lhs, expr_t* rhs);

expr_t* make_unary_op(unary_opt_type_t type, expr_t* arg);

expr_t* make_integer_literal(int value);

expr_t* make_string_literal(char* value);

expr_t* make_variable_use(char* name);

void free_expr(expr_t* expr);

typedef enum {
    BLOCK,
    VARIABLE_DECLARATION,
} statement_type_t;

typedef struct statement {
    statement_type_t type;
    union {
        struct {
            cvector_vector_type(struct statement*) statements;
        } block;
        struct {
            bool constant;
            char* variable_name;
            expr_t* value;
        } variable_declaration;
    } op;
} statement_t;

statement_t* make_block_statement();

statement_t* make_variable_declaration(bool constant, char* variable_name, expr_t* value);

void free_statement(statement_t* statement);

#endif