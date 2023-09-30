#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "mem.h"

expr_t* make_binary_op(binary_opt_type_t type, expr_t* lhs, expr_t* rhs) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_BINARY_OPT;
    expr->op.binary.type = type;
    expr->op.binary.lhs = lhs;
    expr->op.binary.rhs = rhs;
    return expr;
}

expr_t* make_unary_op(unary_opt_type_t type, expr_t* arg) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_UNARY_OPT;
    expr->op.unary.type = type;
    expr->op.unary.arg = arg;
    return expr;
}

expr_t* make_integer_literal(int value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_INT_LITERAL;
    expr->op.integer_literal = value;
    return expr;
}

expr_t* make_string_literal(char* value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_STRING_LITERAL;
    expr->op.string_literal = copy_alloc(value);
    return expr;
}

expr_t* make_variable_use(char* name) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = EXPR_VARIABLE_USE;
    expr->op.variable_use.name = copy_alloc(name);
    return expr;
}

void free_expr(expr_t* expr) {
    switch (expr->type) {
        case EXPR_STRING_LITERAL:
            free(expr->op.string_literal);
            break;
        case EXPR_BINARY_OPT:
            free(expr->op.binary.lhs);
            free(expr->op.binary.rhs);
            break;
        case EXPR_UNARY_OPT:
            free(expr->op.unary.arg);
            break;
        case EXPR_VARIABLE_USE:
            free(expr->op.variable_use.name);
            break;
        default:
            break;
    }
    free(expr);
}

static void vector_statement_deleter(void* element) {
    statement_t* statement = (statement_t*) &element;
    free_statement(statement);
}

statement_t* make_block_statement() {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_BLOCK;
    statement->op.block.statements = NULL;
    cvector_init(statement->op.block.statements, 0, &vector_statement_deleter);
    return statement;
}

statement_t* make_variable_declaration(bool constant, char* variable_name, char* type_name, expr_t* value) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = STATEMENT_VARIABLE_DECL;
    statement->op.variable_declaration.constant = constant;
    statement->op.variable_declaration.variable_name = copy_alloc(variable_name);
    statement->op.variable_declaration.type_name = copy_alloc(type_name);
    statement->op.variable_declaration.value = value;
    return statement;
}

void free_statement(statement_t* statement) {
    switch (statement->type) {
        case STATEMENT_BLOCK:
            cvector_free(statement->op.block.statements);
            break;
        case STATEMENT_VARIABLE_DECL:
            free(statement->op.variable_declaration.variable_name);
            free_expr(statement->op.variable_declaration.value);
            break;
        default:
            break;
    }
    free(statement);
}
