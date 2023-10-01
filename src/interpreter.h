#ifndef PNS_INTERPRETER_INTERPRETER_H
#define PNS_INTERPRETER_INTERPRETER_H

#include "ast.h"
#include "hashmap.h"
#include <stdbool.h>

typedef struct {
    struct hashmap* variables;
} context_t;

typedef enum {
#define PNS_INTERPRETER_RUNTIME_TYPE(A, B) RUNTIME_TYPE_##A,
#include "runtime_types.h"
} runtime_type_t;

typedef struct {
    runtime_type_t type;
    union {
        char* string;
        int integer;
        bool boolean;
        double floating;
    } value;
} runtime_value_t;

typedef struct {
    char* name;
    bool is_constant;
    runtime_value_t content;
} runtime_variable_t;

context_t* create_context();
void dump_context(context_t* context);
void destroy_context(context_t* context);

void execute_statement(context_t* context, statement_t* statement);

runtime_value_t evaluate_expr(context_t* context, expr_t* expr);
runtime_value_t evaluate_binary_op(context_t*, binary_op_type_t op_type, expr_t* lhs, expr_t* rhs);
runtime_value_t evaluate_unary_op(context_t*, unary_op_type_t op_type, expr_t* arg);

runtime_type_t string_to_runtime_type(char* str);
char* runtime_type_to_string(runtime_type_t type);

#endif