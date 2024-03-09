#ifndef CHAD_INTERPRETER_AST_H
#define CHAD_INTERPRETER_AST_H

#include "cvector.h"
#include <stdbool.h>

enum expr_type {
    EXPR_BINARY_OPT,
    EXPR_UNARY_OPT,
    EXPR_BOOL_LITERAL,
    EXPR_INT_LITERAL,
    EXPR_FLOAT_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_NULL,
    EXPR_VARIABLE_USE,
    EXPR_FUNCTION_CALL,
};

enum binary_op_type {
#define CHAD_INTERPRETER_BINARY_OP(X, Y) BINARY_OP_##X,
#include "binary_ops.h"
};

inline bool is_arithmetic_binary_op(enum binary_op_type type) {
    return type == BINARY_OP_ADD || type == BINARY_OP_SUB || type == BINARY_OP_MUL || type == BINARY_OP_DIV || type == BINARY_OP_MODULO;
}

inline bool is_logical_binary_op(enum binary_op_type type) {
    return type == BINARY_OP_AND || type == BINARY_OP_OR;
}

inline bool is_comparison_binary_op(enum binary_op_type type) {
    return !is_arithmetic_binary_op(type) && !is_logical_binary_op(type);
}

const char* binary_op_to_symbol(enum binary_op_type op_type);

enum unary_op_type {
#define CHAD_INTERPRETER_UNARY_OP(X, Y) UNARY_OP_##X,
#include "unary_ops.h"
};

const char* unary_op_to_symbol(enum unary_op_type op_type);

struct expr {
    enum expr_type type;
    union {
        long integer_literal;
        char* string_literal;
        bool bool_literal;
        double float_literal;
        struct {
            enum binary_op_type type;
            struct expr* lhs;
            struct expr* rhs;
        } binary;
        struct {
            enum unary_op_type type;
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
};

struct expr* make_binary_op(enum binary_op_type type, struct expr* lhs, struct expr* rhs);
struct expr* make_unary_op(enum unary_op_type type, struct expr* arg);
struct expr* make_bool_literal(bool value);
struct expr* make_integer_literal(long value);
struct expr* make_float_literal(double value);
struct expr* make_string_literal(const char* value);
struct expr* make_null();
struct expr* make_variable_use(const char* name);
struct expr* make_function_call(const char* name);

void dump_expr(struct expr* expr, int indent);

void destroy_expr(struct expr* expr);

enum statement_type {
    STATEMENT_BLOCK,
    STATEMENT_IF_CONDITION,
    STATEMENT_VARIABLE_DECL,
    STATEMENT_FUNCTION_DECL,
    STATEMENT_VARIABLE_ASSIGN,
    STATEMENT_NAKED_FN_CALL,
    STATEMENT_WHILE_LOOP,
    STATEMENT_FOR_LOOP,
    STATEMENT_BREAK,
    STATEMENT_CONTINUE,
    STATEMENT_RETURN,
};

struct statement {
    enum statement_type type;
    union {
        struct {
            cvector_vector_type(struct statement*) statements;
        } block;
        struct {
            struct expr* condition;
            struct statement* body;
            struct statement* body_else;
        } if_condition;
        struct {
            bool is_constant;
            char* variable_name;
            struct expr* value;
        } variable_declaration;
        struct {
            char* fn_name;
            cvector_vector_type(char*) arguments;
            struct statement* body;
        } function_declaration;
        struct {
            char* variable_name;
            struct expr* value;
        } variable_assignment;
        struct {
            struct expr* function_call;
        } naked_fn_call;
        struct {
            struct expr* condition;
            struct statement* body;
        } while_loop;
        struct {
            struct statement* initializer;
            struct expr* condition;
            struct statement* increment;
            struct statement* body;
        } for_loop;
        struct {
            struct expr* value;
        } return_statement;
    } op;
};

struct statement* make_block_statement();
struct statement* make_if_condition_statement(struct expr* condition, struct statement* body);
struct statement* make_variable_declaration(bool constant, const char* variable_name);
struct statement* make_function_declaration(const char* fn_name);
struct statement* make_variable_assignment(const char* variable_name, struct expr* value);
struct statement* make_naked_fn_call(struct expr* function_call);
struct statement* make_while_loop(struct expr* condition, struct statement* body);
struct statement* make_for_loop(struct statement* initializer, struct expr* condition, struct statement* increment, struct statement* body);
struct statement* make_break_statement();
struct statement* make_continue_statement();
struct statement* make_return_statement();

void dump_statement(struct statement* statement, int indent);

void destroy_statement(struct statement* statement);

#endif
