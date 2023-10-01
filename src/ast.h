#ifndef PNS_INTERPRETER_AST_H
#define PNS_INTERPRETER_AST_H

#include "cvector.h"
#include <stdbool.h>

typedef enum {
    EXPR_BINARY_OPT,
    EXPR_UNARY_OPT,
    EXPR_BOOL_LITERAL,
    EXPR_INT_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_VARIABLE_USE,
    EXPR_FUNCTION_CALL,
} expr_type_t;

typedef enum {
    BINARY_OP_ADD,
    BINARY_OP_SUB,
    BINARY_OP_MUL,
    BINARY_OP_DIV,
    BINARY_OP_AND,
    BINARY_OP_OR,
    BINARY_OP_EQUAL,
    BINARY_OP_NOT_EQUAL,
    BINARY_OP_GREATER,
    BINARY_OP_GREATER_EQUAL,
    BINARY_OP_LESS,
    BINARY_OP_LESS_EQUAL,
} binary_opt_type_t;

typedef enum {
    UNARY_OP_NEG,
    UNARY_OP_NOT,
} unary_opt_type_t;

typedef struct expr {
    expr_type_t type;
    union {
        int integer_literal;
        char* string_literal;
        bool bool_literal;
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
        struct {
            char* name;
            cvector_vector_type(struct expr*) arguments;
        } function_call;
    } op;
} expr_t;

expr_t* make_binary_op(binary_opt_type_t type, expr_t* lhs, expr_t* rhs);
expr_t* make_unary_op(unary_opt_type_t type, expr_t* arg);
expr_t* make_bool_literal(bool value);
expr_t* make_integer_literal(int value);
expr_t* make_string_literal(char* value);
expr_t* make_variable_use(char* name);
expr_t* make_function_call(char* name);

void destroy_expr(expr_t* expr);

typedef enum {
    STATEMENT_BLOCK,
    STATEMENT_IF_CONDITION,
    STATEMENT_VARIABLE_DECL,
    STATEMENT_VARIABLE_ASSIGN,
    STATEMENT_NAKED_FN_CALL,
    STATEMENT_WHILE_LOOP,
    STATEMENT_FUNCTION_DEF,
} statement_type_t;

typedef struct statement {
    statement_type_t type;
    union {
        struct {
            cvector_vector_type(struct statement*) statements;
        } block;
        struct {
            expr_t* condition;
            struct statement* body;
            struct statement* body_else;
        } if_condition;
        struct {
            bool is_constant;
            char* variable_name;
            char* type_name;
            expr_t* value;
        } variable_declaration;
        struct {
            char* variable_name;
            expr_t* value;
        } variable_assignment;
        struct {
            expr_t* function_call;
        } naked_fn_call;
        struct {
            expr_t* condition;
            struct statement* body;
        } while_loop;
        struct {
            char* name;
            char* return_type;
            cvector_vector_type(char*) argument_names;
            cvector_vector_type(char*) argument_types;
            struct statement* body;
        } function_definition;
    } op;
} statement_t;

statement_t* make_block_statement();
statement_t* make_if_condition_statement(expr_t* condition, statement_t* body);
statement_t* make_variable_declaration(bool constant, char* variable_name, char* type_name, expr_t* value);
statement_t* make_variable_assignment(char* variable_name, expr_t* value);
statement_t* make_naked_fn_call(expr_t* function_call);
statement_t* make_while_loop(expr_t* condition, statement_t* body);

void destroy_statement(statement_t* statement);

#endif
