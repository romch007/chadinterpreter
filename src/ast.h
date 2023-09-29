#ifndef PNS_INTERPRETER_AST_H
#define PNS_INTERPRETER_AST_H

typedef enum {
    BINARY_OPT,
    UNARY_OPT,
    INT_LITERAL,
    STRING_LITERAL,
    VARIABLE
} expr_type_t;

typedef enum {
    ADD,
    SUB,
    MUL,
    DIV
} binary_opt_type_t;

typedef enum {
    NEG
} unary_opt_type_t;

typedef struct ast_expr {
    expr_type_t type;
    union {
        int integer_literal;
        char* string_literal;
        struct {
            binary_opt_type_t type;
            struct ast_expr* lhs;
            struct ast_expr* rhs;
        } binary;
        struct {
            unary_opt_type_t type;
            struct ast_expr* arg;
        } unary;
    } op;
} ast_expr_t;

ast_expr_t* make_binary_op(binary_opt_type_t type, ast_expr_t* lhs, ast_expr_t* rhs);

ast_expr_t* make_unary_op(unary_opt_type_t type, ast_expr_t* arg);

ast_expr_t* make_integer_literal(int value);

ast_expr_t* make_string_literal(char* value);

void print_ast_expr(ast_expr_t* root_expr);

#endif
