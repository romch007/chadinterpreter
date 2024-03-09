#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "mem.h"

static void string_deleter(void* element) {
    char* str = *(void**) element;
    free(str);
}

static void vector_statement_deleter(void* element) {
    struct statement* statement = *(void**) element;
    destroy_statement(statement);
}

static void vector_expr_deleter(void* element) {
    struct expr* expr = *(void**) element;
    destroy_expr(expr);
}

static int indent_offset = 2;

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(stderr, " ");
    }
}

extern inline bool is_arithmetic_binary_op(enum binary_op_type type);
extern inline bool is_logical_binary_op(enum binary_op_type type);
extern inline bool is_comparison_binary_op(enum binary_op_type type);

const char* binary_op_to_symbol(enum binary_op_type op_type) {
    switch (op_type) {
#define CHAD_INTERPRETER_BINARY_OP(X, Y) \
    case BINARY_OP_##X:                  \
        return #Y;
#include "binary_ops.h"
    }
    return NULL;
}


const char* unary_op_to_symbol(enum unary_op_type
                                       op_type) {
    switch (op_type) {
#define CHAD_INTERPRETER_UNARY_OP(X, Y) \
    case UNARY_OP_##X:                  \
        return #Y;
#include "unary_ops.h"
    }
    return NULL;
}

struct expr* make_binary_op(enum binary_op_type type, struct expr* lhs, struct expr* rhs) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_BINARY_OPT;
    expr->op.binary.type = type;
    expr->op.binary.lhs = lhs;
    expr->op.binary.rhs = rhs;
    return expr;
}

struct expr* make_unary_op(enum unary_op_type type, struct expr* arg) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_UNARY_OPT;
    expr->op.unary.type = type;
    expr->op.unary.arg = arg;
    return expr;
}

struct expr* make_bool_literal(bool value) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_BOOL_LITERAL;
    expr->op.bool_literal = value;
    return expr;
}

struct expr* make_integer_literal(long value) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_INT_LITERAL;
    expr->op.integer_literal = value;
    return expr;
}

struct expr* make_float_literal(double value) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_FLOAT_LITERAL;
    expr->op.float_literal = value;
    return expr;
}

struct expr* make_string_literal(const char* value) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_STRING_LITERAL;
    expr->op.string_literal = xstrdup(value);
    return expr;
}

struct expr* make_null() {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_NULL;
    return expr;
}

struct expr* make_variable_use(const char* name) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_VARIABLE_USE;
    expr->op.variable_use.name = xstrdup(name);
    return expr;
}

struct expr* make_function_call(const char* name) {
    struct expr* expr = xmalloc(sizeof(struct expr));
    expr->type = EXPR_FUNCTION_CALL;
    expr->op.function_call.name = xstrdup(name);
    expr->op.function_call.arguments = NULL;
    return expr;
}

void dump_expr(struct expr* expr, int indent) {
    switch (expr->type) {
        case EXPR_BINARY_OPT:
            print_indent(indent);
            fprintf(stderr, "BinaryOperation\n");
            dump_expr(expr->op.binary.lhs, indent + indent_offset);
            print_indent(indent + indent_offset);
            fprintf(stderr, "%s\n", binary_op_to_symbol(expr->op.binary.type));
            dump_expr(expr->op.binary.rhs, indent + indent_offset);
            break;
        case EXPR_UNARY_OPT:
            print_indent(indent);
            fprintf(stderr, "UnaryOperation\n");
            print_indent(indent + indent_offset);
            fprintf(stderr, "%s\n", unary_op_to_symbol(expr->op.unary.type));
            dump_expr(expr->op.unary.arg, indent + indent_offset);
            break;
        case EXPR_BOOL_LITERAL:
            print_indent(indent);
            fprintf(stderr, "BoolLiteral %s\n", expr->op.bool_literal ? "true" : "false");
            break;
        case EXPR_INT_LITERAL:
            print_indent(indent);
            fprintf(stderr, "IntLiteral %ld\n", expr->op.integer_literal);
            break;
        case EXPR_FLOAT_LITERAL:
            print_indent(indent);
            fprintf(stderr, "FloatLiteral %f\n", expr->op.float_literal);
            break;
        case EXPR_STRING_LITERAL:
            print_indent(indent);
            fprintf(stderr, "StringLiteral \"%s\"\n", expr->op.string_literal);
            break;
        case EXPR_NULL:
            print_indent(indent);
            fprintf(stderr, "NullLiteral\n");
            break;
        case EXPR_VARIABLE_USE:
            print_indent(indent);
            fprintf(stderr, "Variable %s\n", expr->op.variable_use.name);
            break;
        case EXPR_FUNCTION_CALL:
            print_indent(indent);
            fprintf(stderr, "FunctionCall\n");
            print_indent(indent + indent_offset);
            fprintf(stderr, "Identifier %s\n", expr->op.function_call.name);
            if (expr->op.function_call.arguments != NULL) {
                struct expr** arg;
                for (arg = cvector_begin(expr->op.function_call.arguments); arg != cvector_end(expr->op.function_call.arguments); ++arg) {
                    dump_expr(*arg, indent + indent_offset);
                }
            }
            break;
    }
}

void destroy_expr(struct expr* expr) {
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


struct statement* make_block_statement() {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_BLOCK;
    statement->op.block.statements = NULL;
    return statement;
}

struct statement* make_if_condition_statement(struct expr* condition, struct statement* body) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_IF_CONDITION;
    statement->op.if_condition.condition = condition;
    statement->op.if_condition.body = body;
    statement->op.if_condition.body_else = NULL;
    return statement;
}

struct statement* make_variable_declaration(bool constant, const char* variable_name) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_VARIABLE_DECL;
    statement->op.variable_declaration.is_constant = constant;
    statement->op.variable_declaration.variable_name = xstrdup(variable_name);
    statement->op.variable_declaration.value = NULL;
    return statement;
}

struct statement* make_function_declaration(const char* fn_name) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_FUNCTION_DECL;
    statement->op.function_declaration.fn_name = xstrdup(fn_name);
    statement->op.function_declaration.arguments = NULL;
    statement->op.function_declaration.body = NULL;
    return statement;
}

struct statement* make_variable_assignment(const char* variable_name, struct expr* value) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_VARIABLE_ASSIGN;
    statement->op.variable_assignment.variable_name = xstrdup(variable_name);
    statement->op.variable_assignment.value = value;
    return statement;
}

struct statement* make_naked_fn_call(struct expr* function_call) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_NAKED_FN_CALL;
    statement->op.naked_fn_call.function_call = function_call;
    return statement;
}

struct statement* make_while_loop(struct expr* condition, struct statement* body) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_WHILE_LOOP;
    statement->op.while_loop.condition = condition;
    statement->op.while_loop.body = body;
    return statement;
}

struct statement* make_for_loop(struct statement* initializer, struct expr* condition, struct statement* increment, struct statement* body) {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_FOR_LOOP;
    statement->op.for_loop.initializer = initializer;
    statement->op.for_loop.condition = condition;
    statement->op.for_loop.increment = increment;
    statement->op.for_loop.body = body;
    return statement;
}

struct statement* make_break_statement() {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_BREAK;
    return statement;
}

struct statement* make_continue_statement() {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_CONTINUE;
    return statement;
}

struct statement* make_return_statement() {
    struct statement* statement = xmalloc(sizeof(struct statement));
    statement->type = STATEMENT_RETURN;
    statement->op.return_statement.value = NULL;
    return statement;
}

void dump_statement(struct statement* statement, int indent) {
    switch (statement->type) {
        case STATEMENT_BLOCK: {
            print_indent(indent);
            fprintf(stderr, "(Block)\n");
            struct statement** it;
            for (it = cvector_begin(statement->op.block.statements); it != cvector_end(statement->op.block.statements); ++it) {
                dump_statement(*it, indent + indent_offset);
            }
            break;
        }
        case STATEMENT_IF_CONDITION:
            print_indent(indent);
            fprintf(stderr, "IfStatement\n");
            print_indent(indent);
            fprintf(stderr, "If\n");
            dump_expr(statement->op.if_condition.condition, indent + indent_offset);
            dump_statement(statement->op.if_condition.body, indent + indent_offset);
            if (statement->op.if_condition.body_else != NULL) {
                print_indent(indent);
                fprintf(stderr, "Else\n");
                dump_statement(statement->op.if_condition.body_else, indent + indent_offset);
            }
            break;
        case STATEMENT_VARIABLE_DECL:
            print_indent(indent);
            fprintf(stderr, "VariableDeclaration\n");
            print_indent(indent + indent_offset);
            fprintf(stderr, "%s\n", statement->op.variable_declaration.is_constant ? "Const" : "Let");
            print_indent(indent + indent_offset);
            fprintf(stderr, "Identifier %s\n", statement->op.variable_declaration.variable_name);
            dump_expr(statement->op.variable_declaration.value, indent + indent_offset);
            break;
        case STATEMENT_FUNCTION_DECL:
            print_indent(indent);
            fprintf(stderr, "FunctionDeclaration\n");
            print_indent(indent + indent_offset);
            fprintf(stderr, "Identifier %s\n", statement->op.function_declaration.fn_name);

            if (statement->op.function_declaration.arguments != NULL) {
                print_indent(indent + indent_offset);
                fprintf(stderr, "Arguments ");
                char** arg_name;
                for (arg_name = cvector_begin(statement->op.function_declaration.arguments); arg_name != cvector_end(statement->op.function_declaration.arguments); ++arg_name) {
                    fprintf(stderr, "%s ", *arg_name);
                }
                fprintf(stderr, "\n");
            }

            dump_statement(statement->op.function_declaration.body, indent + indent_offset);
            break;
        case STATEMENT_VARIABLE_ASSIGN:
            print_indent(indent);
            fprintf(stderr, "VariableAssignment\n");
            print_indent(indent + indent_offset);
            fprintf(stderr, "Identifier %s\n", statement->op.variable_assignment.variable_name);
            dump_expr(statement->op.variable_assignment.value, indent + indent_offset);
            break;
        case STATEMENT_NAKED_FN_CALL:
            print_indent(indent);
            fprintf(stderr, "NakedFunctionCall\n");
            dump_expr(statement->op.naked_fn_call.function_call, indent + indent_offset);
            break;
        case STATEMENT_WHILE_LOOP:
            print_indent(indent);
            fprintf(stderr, "While\n");
            dump_expr(statement->op.while_loop.condition, indent + indent_offset);
            dump_statement(statement->op.while_loop.body, indent + indent_offset);
            break;
        case STATEMENT_FOR_LOOP:
            print_indent(indent);
            fprintf(stderr, "For\n");
            dump_statement(statement->op.for_loop.initializer, indent + indent_offset);
            dump_expr(statement->op.for_loop.condition, indent + indent_offset);
            dump_statement(statement->op.for_loop.increment, indent + indent_offset);
            dump_statement(statement->op.for_loop.body, indent + indent_offset);
            break;
        case STATEMENT_BREAK:
            print_indent(indent);
            fprintf(stderr, "BreakStatement\n");
            break;
        case STATEMENT_CONTINUE:
            print_indent(indent);
            fprintf(stderr, "ContinueStatement\n");
            break;
        case STATEMENT_RETURN:
            print_indent(indent);
            fprintf(stderr, "ReturnStatement\n");
            if (statement->op.return_statement.value != NULL) {
                dump_expr(statement->op.return_statement.value, indent + indent_offset);
            }
            break;
    }
}

void destroy_statement(struct statement* statement) {
    if (statement == NULL) return;

    switch (statement->type) {
        case STATEMENT_BLOCK:
            cvector_set_elem_destructor(statement->op.block.statements, vector_statement_deleter);
            cvector_free(statement->op.block.statements);
            break;
        case STATEMENT_VARIABLE_DECL:
            free(statement->op.variable_declaration.variable_name);
            destroy_expr(statement->op.variable_declaration.value);
            break;
        case STATEMENT_FUNCTION_DECL:
            free(statement->op.function_declaration.fn_name);
            destroy_statement(statement->op.function_declaration.body);
            cvector_set_elem_destructor(statement->op.function_declaration.arguments, string_deleter);
            cvector_free(statement->op.function_declaration.arguments);
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
        case STATEMENT_FOR_LOOP:
            destroy_statement(statement->op.for_loop.initializer);
            destroy_expr(statement->op.for_loop.condition);
            destroy_statement(statement->op.for_loop.increment);
            destroy_statement(statement->op.for_loop.body);
            break;
        case STATEMENT_RETURN:
            destroy_expr(statement->op.return_statement.value);
        default:
            break;
    }

    free(statement);
}
