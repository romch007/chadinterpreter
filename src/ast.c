#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "mem.h"

expr_t* make_binary_op(binary_opt_type_t type, expr_t* lhs, expr_t* rhs) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = BINARY_OPT;
    expr->op.binary.type = type;
    expr->op.binary.lhs = lhs;
    expr->op.binary.rhs = rhs;
    return expr;
}

expr_t* make_unary_op(unary_opt_type_t type, expr_t* arg) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = UNARY_OPT;
    expr->op.unary.type = type;
    expr->op.unary.arg = arg;
    return expr;
}

expr_t* make_integer_literal(int value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = INT_LITERAL;
    expr->op.integer_literal = value;
    return expr;
}

expr_t* make_string_literal(char* value) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = STRING_LITERAL;
    char* in_value = xmalloc(sizeof(char) * strlen(value));
    strcpy(in_value, value);
    expr->op.string_literal = in_value;
    return expr;
}

expr_t* make_variable_use(char* name) {
    expr_t* expr = xmalloc(sizeof(expr_t));
    expr->type = VARIABLE_USE;
    char* in_name = xmalloc(sizeof(char) * strlen(name));
    strcpy(in_name, name);
    expr->op.variable_use.name = in_name;
    return expr;
}

void free_expr(expr_t* expr) {
    switch (expr->type) {
        case STRING_LITERAL:
            free(expr->op.string_literal);
            break;
        case BINARY_OPT:
            free(expr->op.binary.lhs);
            free(expr->op.binary.rhs);
            break;
        case UNARY_OPT:
            free(expr->op.unary.arg);
            break;
        case VARIABLE_USE:
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
    statement->type = BLOCK;
    statement->op.block.statements = NULL;
    cvector_init(statement->op.block.statements, 0, &vector_statement_deleter); // NOLINT(*-sizeof-expression)
    return statement;
}

statement_t* make_variable_declaration(bool constant, char* variable_name, expr_t* value) {
    statement_t* statement = xmalloc(sizeof(statement_t));
    statement->type = VARIABLE_DECLARATION;
    statement->op.variable_declaration.constant = constant;
    char* in_variable_name = xmalloc(sizeof(char) * strlen(variable_name));
    strcpy(in_variable_name, variable_name);
    statement->op.variable_declaration.variable_name = in_variable_name;
    statement->op.variable_declaration.value = value;
    return statement;
}

void free_statement(statement_t* statement) {
    switch (statement->type) {
        case BLOCK:
            cvector_free(statement->op.block.statements);
            break;
        case VARIABLE_DECLARATION:
            free(statement->op.variable_declaration.variable_name);
            free_expr(statement->op.variable_declaration.value);
            break;
        default:
            break;
    }
    free(statement);
}
