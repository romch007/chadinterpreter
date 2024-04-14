#ifndef CHAD_INTERPRETER_INTERPRETER_H
#define CHAD_INTERPRETER_INTERPRETER_H

#include <stdbool.h>

#include "ast.h"
#include "gc.h"
#include "hashmap.h"

static const int MAX_RECURSION_DEPTH = 1000;

enum runtime_type {
#define CHAD_INTERPRETER_RUNTIME_TYPE(A, B) RUNTIME_TYPE_##A,
#include "runtime_types.h"
};

struct runtime_value {
    enum runtime_type type;
    union {
        struct ref_counted string;
        long integer;
        bool boolean;
        double floating;
    } value;
};

struct runtime_variable {
    char* name;
    bool is_constant;
    struct runtime_value content;
};

struct stack_frame {
    struct hashmap* variables;
    struct hashmap* functions;
};

struct context {
    struct stack_frame* frames;
    bool should_break_loop;
    bool should_continue_loop;
    bool should_return_fn;
    bool has_return_value;
    struct runtime_value return_value;
    int recursion_depth;
};

struct context* create_context();
void destroy_context(struct context* context);

void push_stack_frame(struct context* context);
void pop_stack_frame(struct context* context);

void dump_context(struct context* context);
void dump_stack_frame(struct stack_frame* frame);

void print_value(const struct runtime_value* value);
void destroy_value(const struct runtime_value* value);

const struct runtime_variable* get_variable(struct context* context, const char* variable_name, int* stack_index);
const struct statement* get_function(struct context* context, const char* fn_name, int* stack_index);

void execute_statement(struct context* context, struct statement* statement);
void execute_variable_declaration(struct context* context, struct statement* statement);
void execute_variable_assignment(struct context* context, struct statement* statement);

struct runtime_value evaluate_expr(struct context* context, struct expr* expr);
struct runtime_value evaluate_binary_op(struct context*, enum binary_op_type op_type, struct expr* lhs, struct expr* rhs);
struct runtime_value evaluate_unary_op(struct context*, enum unary_op_type op_type, struct expr* arg);
struct runtime_value evaluate_function_call(struct context* context, const char* fn_name, struct expr** arguments);

enum runtime_type string_to_runtime_type(const char* str);
const char* runtime_type_to_string(enum runtime_type type);

#endif
