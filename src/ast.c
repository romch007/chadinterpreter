#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "mem.h"

ast_expr_t* make_binary_op(binary_opt_type_t type, ast_expr_t* lhs, ast_expr_t* rhs) {
    ast_expr_t* expr = xmalloc(sizeof(ast_expr_t));
    expr->type = BINARY_OPT;
    expr->op.binary.type = type;
    expr->op.binary.lhs = lhs;
    expr->op.binary.rhs = rhs;
    return expr;
}

ast_expr_t* make_unary_op(unary_opt_type_t type, ast_expr_t* arg) {
    ast_expr_t* expr = xmalloc(sizeof(ast_expr_t));
    expr->type = UNARY_OPT;
    expr->op.unary.type = type;
    expr->op.unary.arg = arg;
    return expr;
}

ast_expr_t* make_integer_literal(int value) {
    ast_expr_t* expr = xmalloc(sizeof(ast_expr_t));
    expr->type = INT_LITERAL;
    expr->op.integer_literal = value;
    return expr;
}

ast_expr_t* make_string_literal(char* value) {
    ast_expr_t* expr = xmalloc(sizeof(ast_expr_t));
    expr->type = INT_LITERAL;
    char* in_value = xmalloc(sizeof(char) * strlen(value));
    strcpy(in_value, value);
    expr->op.string_literal = in_value;
    return expr;
}

void print_ast_expr(ast_expr_t* root_expr) {
}
