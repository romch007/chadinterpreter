#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "mem.h"

static void vector_statement_deleter(void* element) {
    statement_t* statement = *(void**) element;
    destroy_statement(statement);
}

static void vector_expr_deleter(void* element) {
    expr_t* expr = *(void**) element;
    destroy_expr(expr);
}


extern inline bool is_arithmetic_binary_op(binary_op_type_t type);
extern inline bool is_logical_binary_op(binary_op_type_t type);
extern inline bool is_comparison_binary_op(binary_op_type_t type);

expr_t* make_binary_op(binary_op_type_t type, expr_t* lhs, expr_t* rhs) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_BINARY_OPT;
    expr->op.binary.type = type;
    expr->op.binary.lhs = lhs;
    expr->op.binary.rhs = rhs;
    return expr;
}

expr_t* make_unary_op(unary_op_type_t type, expr_t* arg) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_UNARY_OPT;
    expr->op.unary.type = type;
    expr->op.unary.arg = arg;
    return expr;
}

expr_t* make_bool_literal(bool value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_BOOL_LITERAL;
    expr->op.bool_literal = value;
    return expr;
}

expr_t* make_integer_literal(int value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_INT_LITERAL;
    expr->op.integer_literal = value;
    return expr;
}

expr_t* make_float_literal(double value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_FLOAT_LITERAL;
    expr->op.float_literal = value;
    return expr;
}

expr_t* make_string_literal(const char* value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_STRING_LITERAL;
    expr->op.string_literal = copy_alloc(value);
    return expr;
}

expr_t* make_variable_use(const char* name) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_VARIABLE_USE;
    expr->op.variable_use.name = copy_alloc(name);
    return expr;
}

expr_t* make_function_call(const char* name) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_FUNCTION_CALL;
    expr->op.function_call.name = copy_alloc(name);
    expr->op.function_call.arguments = NULL;
    return expr;
}

void destroy_expr(expr_t* expr) {
    if (expr == NULL) return;

    switch (expr->type) {
        case EXPR_STRING_LITERAL:
            free(expr->op.string_literal);
            break;
        case EXPR_BINARY_OPT:
            destroy_expr(expr->op.binary.lhs);
            destroy_expr(expr->op.binary.rhs);
            break;
        case EXPR_UNARY_OPT:
            destroy_expr(expr->op.unary.arg);
            break;
        case EXPR_VARIABLE_USE:
            free(expr->op.variable_use.name);
            break;
        case EXPR_FUNCTION_CALL:
            free(expr->op.function_call.name);
            cvector_set_elem_destructor(expr->op.function_call.arguments, vector_expr_deleter);
            cvector_free(expr->op.function_call.arguments);
            break;
        default:
            break;
    }

    free(expr);
}


statement_t* make_block_statement() {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_BLOCK;
    statement->op.block.statements = NULL;
    return statement;
}

statement_t* make_if_condition_statement(expr_t* condition, statement_t* body) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_IF_CONDITION;
    statement->op.if_condition.condition = condition;
    statement->op.if_condition.body = body;
    statement->op.if_condition.body_else = NULL;
    return statement;
}

statement_t* make_variable_declaration(bool constant, const char* variable_name) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_VARIABLE_DECL;
    statement->op.variable_declaration.is_constant = constant;
    statement->op.variable_declaration.variable_name = copy_alloc(variable_name);
    statement->op.variable_declaration.type_name = NULL;
    statement->op.variable_declaration.value = NULL;
    return statement;
}

statement_t* make_variable_assignment(const char* variable_name, expr_t* value) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_VARIABLE_ASSIGN;
    statement->op.variable_assignment.variable_name = copy_alloc(variable_name);
    statement->op.variable_assignment.value = value;
    return statement;
}

statement_t* make_naked_fn_call(expr_t* function_call) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_NAKED_FN_CALL;
    statement->op.naked_fn_call.function_call = function_call;
    return statement;
}

statement_t* make_while_loop(expr_t* condition, statement_t* body) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_WHILE_LOOP;
    statement->op.while_loop.condition = condition;
    statement->op.while_loop.body = body;
    return statement;
}

void destroy_statement(statement_t* statement) {
    if (statement == NULL) return;

    switch (statement->type) {
        case STATEMENT_BLOCK:
            cvector_set_elem_destructor(statement->op.block.statements, vector_statement_deleter);
            cvector_free(statement->op.block.statements);
            break;
        case STATEMENT_VARIABLE_DECL:
            free(statement->op.variable_declaration.variable_name);
            free(statement->op.variable_declaration.type_name);
            destroy_expr(statement->op.variable_declaration.value);
            break;
        case STATEMENT_IF_CONDITION:
            destroy_expr(statement->op.if_condition.condition);
            destroy_statement(statement->op.if_condition.body);
            destroy_statement(statement->op.if_condition.body_else);
            break;
        case STATEMENT_VARIABLE_ASSIGN:
            free(statement->op.variable_assignment.variable_name);
            destroy_expr(statement->op.variable_assignment.value);
            break;
        case STATEMENT_NAKED_FN_CALL:
            destroy_expr(statement->op.naked_fn_call.function_call);
            break;
        case STATEMENT_WHILE_LOOP:
            destroy_expr(statement->op.while_loop.condition);
            destroy_statement(statement->op.while_loop.body);
            break;
        default:
            break;
    }

    free(statement);
}
