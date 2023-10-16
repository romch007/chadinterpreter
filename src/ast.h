#ifndef CHAD_INTERPRETER_AST_H
#define CHAD_INTERPRETER_AST_H

#include "cvector.h"
#include <stdbool.h>

typedef enum {
    EXPR_BINARY_OPT,
    EXPR_UNARY_OPT,
    EXPR_BOOL_LITERAL,
    EXPR_INT_LITERAL,
    EXPR_FLOAT_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_VARIABLE_USE,
    EXPR_FUNCTION_CALL,
} expr_type_t;

typedef enum {
#define CHAD_INTERPRETER_BINARY_OP(X, Y) BINARY_OP_##X,
#include "binary_ops.h"
} binary_op_type_t;

inline bool is_arithmetic_binary_op(binary_op_type_t type) {
    return type == BINARY_OP_ADD || type == BINARY_OP_SUB || type == BINARY_OP_MUL || type == BINARY_OP_DIV || type == BINARY_OP_MODULO;
}

inline bool is_logical_binary_op(binary_op_type_t type) {
    return type == BINARY_OP_AND || type == BINARY_OP_OR;
}

inline bool is_comparison_binary_op(binary_op_type_t type) {
    return !is_arithmetic_binary_op(type) && !is_logical_binary_op(type);
}

const char* binary_op_to_symbol(binary_op_type_t op_type);

typedef enum {
#define CHAD_INTERPRETER_UNARY_OP(X, Y) UNARY_OP_##X,
#include "unary_ops.h"
} unary_op_type_t;

const char* unary_op_to_symbol(unary_op_type_t op_type);

typedef struct expr {
    expr_type_t type;
    union {
        int integer_literal;
        char* string_literal;
        bool bool_literal;
        double float_literal;
        struct {
            binary_op_type_t type;
            struct expr* lhs;
            struct expr* rhs;
        } binary;
        struct {
            unary_op_type_t type;
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

expr_t* make_binary_op(binary_op_type_t type, expr_t* lhs, expr_t* rhs);
expr_t* make_unary_op(unary_op_type_t type, expr_t* arg);
expr_t* make_bool_literal(bool value);
expr_t* make_integer_literal(int value);
expr_t* make_float_literal(double value);
expr_t* make_string_literal(const char* value);
expr_t* make_variable_use(const char* name);
expr_t* make_function_call(const char* name);

void dump_expr(expr_t* expr, int indent);

void destroy_expr(expr_t* expr);

typedef enum {
    STATEMENT_BLOCK,
    STATEMENT_IF_CONDITION,
    STATEMENT_VARIABLE_DECL,
    STATEMENT_FUNCTION_DECL,
    STATEMENT_VARIABLE_ASSIGN,
    STATEMENT_NAKED_FN_CALL,
    STATEMENT_WHILE_LOOP,
    STATEMENT_BREAK,
    STATEMENT_CONTINUE,
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
            expr_t* value;
        } variable_declaration;
        struct {
            char* fn_name;
            cvector_vector_type(char*) arguments;
            struct statement* body;
        } function_declaration;
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
    } op;
} statement_t;

statement_t* make_block_statement();
statement_t* make_if_condition_statement(expr_t* condition, statement_t* body);
statement_t* make_variable_declaration(bool constant, const char* variable_name);
statement_t* make_function_declaration(const char* fn_name);
statement_t* make_variable_assignment(const char* variable_name, expr_t* value);
statement_t* make_naked_fn_call(expr_t* function_call);
statement_t* make_while_loop(expr_t* condition, statement_t* body);
statement_t* make_break_statement();
statement_t* make_continue_statement();

void dump_statement(statement_t* statement, int indent);

void destroy_statement(statement_t* statement);

#endif
