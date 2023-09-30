#include "debug.h"
#include <stdio.h>

static void pad(int padding) {
    if (padding == 0) return;
    printf("%*c", padding, ' ');
}

void print_expression(expr_t* expr, int padding) {
    switch (expr->type) {
        case EXPR_INT_LITERAL:
            pad(padding);
            printf("IntLiteral(%d)\n", expr->op.integer_literal);
            break;
        case EXPR_STRING_LITERAL:
            pad(padding);
            printf("StringLiteral(%s)\n", expr->op.string_literal);
            break;
        case EXPR_VARIABLE_USE:
            pad(padding);
            printf("Variable(%s)\n", expr->op.variable_use.name);
            break;
        case EXPR_BINARY_OPT:
            pad(padding);
            printf("BinaryOp(\n");
            pad(padding + 2);
            printf("type: %d\n", expr->op.binary.type);
            pad(padding + 2);
            printf("lhs:\n");
            print_expression(expr->op.binary.lhs, padding + 4);
            pad(padding + 2);
            printf("rhs:\n");
            print_expression(expr->op.binary.rhs, padding + 4);
            pad(padding);
            printf(")\n");
            break;
        case EXPR_UNARY_OPT:
            pad(padding);
            printf("UnaryOp(\n");
            pad(padding + 2);
            printf("type: %d\n", expr->op.unary.type);
            pad(padding + 2);
            printf("arg:\n");
            print_expression(expr->op.unary.arg, padding + 4);
            pad(padding);
            printf(")\n");
            break;
    }
}