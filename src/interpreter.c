#include "interpreter.h"
#include "mem.h"

static int variable_compare(const void* a, const void* b, void* udata) {
    const runtime_variable_t* va = a;
    const runtime_variable_t* vb = b;

    return strcmp(va->name, vb->name);
}

static uint64_t variable_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const runtime_variable_t* v = item;
    return hashmap_sip(v->name, strlen(v->name), seed0, seed1);
}

context_t* create_context() {
    context_t* context = xmalloc(sizeof(context));
    context->variables = hashmap_new(sizeof(runtime_variable_t), 0, 0, 0, variable_hash, variable_compare, NULL, NULL);
    return context;
}

void dump_context(context_t* context) {
    printf("--- variables ---\n");
    size_t iter = 0;
    void* item;
    while (hashmap_iter(context->variables, &iter, &item)) {
        const runtime_variable_t* variable = item;
        printf("%s: %s = ", variable->name, runtime_type_to_string(variable->content.type));
        switch (variable->content.type) {
            case RUNTIME_TYPE_STRING:
                printf("%s\n", variable->content.value.string ? variable->content.value.string : "(empty)");
                break;
            case RUNTIME_TYPE_INTEGER:
                printf("%d\n", variable->content.value.integer);
                break;
            case RUNTIME_TYPE_FLOAT:
                printf("%f\n", variable->content.value.floating);
                break;
            case RUNTIME_TYPE_BOOLEAN:
                printf("%s\n", variable->content.value.boolean ? "true" : "false");
                break;
        }
    }
}

void destroy_context(context_t* context) {
    size_t iter = 0;
    void* item;
    while (hashmap_iter(context->variables, &iter, &item)) {
        const runtime_variable_t* variable = item;
        free(variable->name);
        if (variable->content.type == RUNTIME_TYPE_STRING) {
            free(variable->content.value.string);
        }
    }
    hashmap_free(context->variables);
    free(context);
}

void execute_statement(context_t* context, statement_t* statement) {
    switch (statement->type) {
        case STATEMENT_BLOCK: {
            cvector_vector_type(statement_t*) statements = statement->op.block.statements;

            statement_t** it;
            for (it = cvector_begin(statements); it != cvector_end(statements); ++it) {
                statement_t* current_statement = *it;
                execute_statement(context, current_statement);
            }

            break;
        }
        case STATEMENT_VARIABLE_DECL: {
            runtime_variable_t variable = {
                    .name = copy_alloc(statement->op.variable_declaration.variable_name),
                    .is_constant = statement->op.variable_declaration.is_constant,
            };

            char* type_name = statement->op.variable_declaration.type_name;

            if (type_name != NULL) {
                runtime_type_t default_value_type = string_to_runtime_type(type_name);

                if (default_value_type == -1) {
                    printf("ERROR: invalid type '%s'\n", type_name);
                    exit(EXIT_FAILURE);
                }

                variable.content.type = default_value_type;

                switch (default_value_type) {
                    case RUNTIME_TYPE_STRING:
                        variable.content.value.string = NULL;
                        break;
                    case RUNTIME_TYPE_INTEGER:
                        variable.content.value.integer = 0;
                        break;
                    case RUNTIME_TYPE_FLOAT:
                        variable.content.value.floating = 0.0;
                        break;
                    case RUNTIME_TYPE_BOOLEAN:
                        variable.content.value.boolean = false;
                        break;
                }
            } else {
                runtime_value_t default_value = evaluate_expr(context, statement->op.variable_declaration.value);

                variable.content = default_value;
            }

            hashmap_set(context->variables, &variable);
            break;
        }
        case STATEMENT_VARIABLE_ASSIGN: {
            char* variable_name = statement->op.variable_assignment.variable_name;

            const runtime_variable_t* old_variable = hashmap_get(context->variables, &(runtime_variable_t){.name = variable_name});

            if (old_variable->is_constant) {
                printf("ERROR: variable '%s' is constant\n", variable_name);
                exit(EXIT_FAILURE);
            }

            runtime_value_t new_content = evaluate_expr(context, statement->op.variable_assignment.value);

            if (old_variable->content.type != new_content.type) {
                printf("ERROR: cannot assign value of type %s to variable '%s' of type %s\n", runtime_type_to_string(new_content.type), variable_name, runtime_type_to_string(old_variable->content.type));
                exit(EXIT_FAILURE);
            }

            runtime_variable_t variable = {
                    .name = old_variable->name,
                    .content = new_content,
                    .is_constant = false,
            };

            hashmap_set(context->variables, &variable);
            break;
        }
        case STATEMENT_IF_CONDITION: {
            runtime_value_t condition = evaluate_expr(context, statement->op.if_condition.condition);

            if (condition.type != RUNTIME_TYPE_BOOLEAN) {
                printf("ERROR: found a value of type %s in a if condition\n", runtime_type_to_string(condition.type));
                exit(EXIT_FAILURE);
            }

            statement_t* body = statement->op.if_condition.body;
            statement_t* body_else = statement->op.if_condition.body_else;

            if (condition.value.boolean) {
                execute_statement(context, body);
            } else if (body_else != NULL) {
                execute_statement(context, body_else);
            }
            break;
        }
        case STATEMENT_WHILE_LOOP: {
            runtime_value_t condition = evaluate_expr(context, statement->op.while_loop.condition);

            if (condition.type != RUNTIME_TYPE_BOOLEAN) {
                printf("ERROR: found a value of type %s in a while condition\n", runtime_type_to_string(condition.type));
                exit(EXIT_FAILURE);
            }

            while (condition.value.boolean) {
                execute_statement(context, statement->op.while_loop.body);

                condition = evaluate_expr(context, statement->op.while_loop.condition);
            }
            break;
        }
        default:
            printf("ERROR: cannot execute statement\n");
            abort();
    }
}

runtime_value_t evaluate_expr(context_t* context, expr_t* expr) {
    switch (expr->type) {
        case EXPR_BOOL_LITERAL: {
            runtime_value_t value = {
                    .type = RUNTIME_TYPE_BOOLEAN,
                    .value.boolean = expr->op.bool_literal};

            return value;
        }
        case EXPR_INT_LITERAL: {
            runtime_value_t value = {
                    .type = RUNTIME_TYPE_INTEGER,
                    .value.integer = expr->op.integer_literal};

            return value;
        }
        case EXPR_FLOAT_LITERAL: {
            runtime_value_t value = {
                    .type = RUNTIME_TYPE_FLOAT,
                    .value.floating = expr->op.float_literal};

            return value;
        }
        case EXPR_STRING_LITERAL: {
            runtime_value_t value = {
                    .type = RUNTIME_TYPE_STRING,
                    .value.string = copy_alloc(expr->op.string_literal)};

            return value;
        }
        case EXPR_VARIABLE_USE: {
            char* variable_name = expr->op.variable_use.name;

            const runtime_variable_t* variable = hashmap_get(context->variables, &(runtime_variable_t){.name = variable_name});

            if (variable == NULL) {
                printf("ERROR: cannot find variable '%s'\n", variable_name);
                exit(EXIT_FAILURE);
            }

            return variable->content;
        }
        case EXPR_BINARY_OPT:
            return evaluate_binary_op(context, expr->op.binary.type, expr->op.binary.lhs, expr->op.binary.rhs);
        case EXPR_UNARY_OPT:
            return evaluate_unary_op(context, expr->op.unary.type, expr->op.unary.arg);
        default:
            printf("ERROR: cannot evaluate expression\n");
            abort();
    }
}

runtime_value_t evaluate_binary_op(context_t* context, binary_op_type_t op_type, expr_t* lhs, expr_t* rhs) {
    runtime_value_t lhs_value = evaluate_expr(context, lhs);
    runtime_value_t rhs_value = evaluate_expr(context, rhs);

    if (lhs_value.type != rhs_value.type) {
        printf("ERROR: type mismatch between %s and %s\n", runtime_type_to_string(lhs_value.type), runtime_type_to_string(rhs_value.type));
        exit(EXIT_FAILURE);
    }

    runtime_type_t value_type = lhs_value.type;

    if (is_arithmetic_binary_op(op_type)) {
        if (value_type != RUNTIME_TYPE_INTEGER && value_type != RUNTIME_TYPE_FLOAT) {
            printf("ERROR: cannot use arithmetic operator on type %s\n", runtime_type_to_string(value_type));
            exit(EXIT_FAILURE);
        }

        if (value_type == RUNTIME_TYPE_INTEGER) {
            // Arithmetic operations with integers
            runtime_value_t result_value = {
                    .type = RUNTIME_TYPE_INTEGER};

            switch (op_type) {
                case BINARY_OP_ADD:
                    result_value.value.integer = lhs_value.value.integer + rhs_value.value.integer;
                    break;
                case BINARY_OP_SUB:
                    result_value.value.integer = lhs_value.value.integer - rhs_value.value.integer;
                    break;
                case BINARY_OP_MUL:
                    result_value.value.integer = lhs_value.value.integer * rhs_value.value.integer;
                    break;
                case BINARY_OP_DIV:
                    if (rhs_value.value.integer == 0) {
                        printf("ERROR: cannot divide by zero\n");
                        exit(EXIT_FAILURE);
                    }
                    result_value.value.integer = lhs_value.value.integer / rhs_value.value.integer;
                    break;
                default:
                    break;
            }

            return result_value;
        } else {
            // Arithmetic operations with floats
            runtime_value_t result_value = {
                    .type = RUNTIME_TYPE_FLOAT};

            switch (op_type) {
                case BINARY_OP_ADD:
                    result_value.value.floating = lhs_value.value.floating + rhs_value.value.floating;
                    break;
                case BINARY_OP_SUB:
                    result_value.value.floating = lhs_value.value.floating - rhs_value.value.floating;
                    break;
                case BINARY_OP_MUL:
                    result_value.value.floating = lhs_value.value.floating * rhs_value.value.floating;
                    break;
                case BINARY_OP_DIV:
                    if (rhs_value.value.floating == 0) {
                        printf("ERROR: cannot divide by zero\n");
                        exit(EXIT_FAILURE);
                    }
                    result_value.value.floating = lhs_value.value.floating / rhs_value.value.floating;
                    break;
                default:
                    break;
            }

            return result_value;
        }
    } else if (is_logical_binary_op(op_type)) {
        if (value_type != RUNTIME_TYPE_BOOLEAN) {
            printf("ERROR: cannot use logical operator on type %s\n", runtime_type_to_string(value_type));
            exit(EXIT_FAILURE);
        }

        runtime_value_t result_value = {
                .type = RUNTIME_TYPE_BOOLEAN,
        };

        switch (op_type) {
            case BINARY_OP_AND:
                result_value.value.boolean = lhs_value.value.boolean && rhs_value.value.boolean;
                break;
            case BINARY_OP_OR:
                result_value.value.boolean = lhs_value.value.boolean || rhs_value.value.boolean;
                break;
            default:
                break;
        }

        return result_value;
    } else if (is_comparison_binary_op(op_type)) {
        if (value_type != RUNTIME_TYPE_INTEGER && value_type != RUNTIME_TYPE_FLOAT) {
            printf("ERROR: cannot use comparison operator on type %s\n", runtime_type_to_string(value_type));
            exit(EXIT_FAILURE);
        }

        runtime_value_t result_value = {
                .type = RUNTIME_TYPE_BOOLEAN};

        if (value_type == RUNTIME_TYPE_INTEGER) {
            // Comparison operations with integers
            switch (op_type) {
                case BINARY_OP_EQUAL:
                    result_value.value.boolean = lhs_value.value.integer == rhs_value.value.integer;
                    break;
                case BINARY_OP_NOT_EQUAL:
                    result_value.value.boolean = lhs_value.value.integer != rhs_value.value.integer;
                    break;
                case BINARY_OP_GREATER:
                    result_value.value.boolean = lhs_value.value.integer > rhs_value.value.integer;
                    break;
                case BINARY_OP_GREATER_EQUAL:
                    result_value.value.boolean = lhs_value.value.integer >= rhs_value.value.integer;
                    break;
                case BINARY_OP_LESS:
                    result_value.value.boolean = lhs_value.value.integer < rhs_value.value.integer;
                    break;
                case BINARY_OP_LESS_EQUAL:
                    result_value.value.boolean = lhs_value.value.integer <= rhs_value.value.integer;
                    break;
                default:
                    break;
            }
        } else {
            // Comparison operations with floats
            switch (op_type) {
                case BINARY_OP_EQUAL:
                    result_value.value.boolean = lhs_value.value.floating == rhs_value.value.floating;
                    break;
                case BINARY_OP_NOT_EQUAL:
                    result_value.value.boolean = lhs_value.value.floating != rhs_value.value.floating;
                    break;
                case BINARY_OP_GREATER:
                    result_value.value.boolean = lhs_value.value.floating > rhs_value.value.floating;
                    break;
                case BINARY_OP_GREATER_EQUAL:
                    result_value.value.boolean = lhs_value.value.floating >= rhs_value.value.floating;
                    break;
                case BINARY_OP_LESS:
                    result_value.value.boolean = lhs_value.value.floating < rhs_value.value.floating;
                    break;
                case BINARY_OP_LESS_EQUAL:
                    result_value.value.boolean = lhs_value.value.floating <= rhs_value.value.floating;
                    break;
                default:
                    break;
            }
        }

        return result_value;
    } else {
        printf("ERROR: unknown binary operator\n");
        abort();
    }
}

runtime_value_t evaluate_unary_op(context_t* context, unary_op_type_t op_type, expr_t* arg) {
    runtime_value_t arg_value = evaluate_expr(context, arg);

    if (op_type == UNARY_OP_NOT) {
        if (arg_value.type != RUNTIME_TYPE_BOOLEAN) {
            printf("ERROR: cannot use logical operation on type %s\n", runtime_type_to_string(arg_value.type));
            exit(EXIT_FAILURE);
        }

        arg_value.value.boolean = !arg_value.value.boolean;

        return arg_value;
    } else if (op_type == UNARY_OP_NEG) {
        if (arg_value.type != RUNTIME_TYPE_INTEGER && arg_value.type != RUNTIME_TYPE_FLOAT) {
            printf("ERROR: cannot use logical operation on type %s\n", runtime_type_to_string(arg_value.type));
            exit(EXIT_FAILURE);
        }

        switch (arg_value.type) {
            case RUNTIME_TYPE_FLOAT:
                arg_value.value.floating = -arg_value.value.floating;
                break;
            case RUNTIME_TYPE_INTEGER:
                arg_value.value.integer = -arg_value.value.integer;
                break;
            default:
                break;
        }

        return arg_value;
    } else {
        printf("ERROR: unknown unary operator\n");
        abort();
    }
}

runtime_type_t string_to_runtime_type(char* str) {
#define PNS_INTERPRETER_RUNTIME_TYPE(A, B) \
    if (strcmp(str, #B) == 0) {                 \
        return RUNTIME_TYPE_##A;           \
    }
#include "runtime_types.h"
    return -1;
}

char* runtime_type_to_string(runtime_type_t type) {
    switch (type) {
#define PNS_INTERPRETER_RUNTIME_TYPE(A, B) \
    case RUNTIME_TYPE_##A:                 \
        return #B;
#include "runtime_types.h"
    }
}