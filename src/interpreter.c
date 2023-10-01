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
        printf("%s - type %s\n", variable->name, runtime_type_to_string(variable->value.type));
    }
}

void destroy_context(context_t* context) {
    size_t iter = 0;
    void* item;
    while (hashmap_iter(context->variables, &iter, &item)) {
        const runtime_variable_t* variable = item;
        free(variable->name);
        if (variable->value.type == RUNTIME_TYPE_STRING) {
            free(variable->value.value.string);
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
                    .value = evaluate_expr(context, statement->op.variable_declaration.value),
                    .is_constant = statement->op.variable_declaration.is_constant};

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

            runtime_variable_t variable = {
                    .name = old_variable->name,
                    .value = evaluate_expr(context, statement->op.variable_assignment.value),
                    .is_constant = false,
            };

            hashmap_set(context->variables, &variable);
        }
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

            return variable->value;
        }
        case EXPR_BINARY_OPT:
            return evaluate_binary_op(context, expr->op.binary.type, expr->op.binary.lhs, expr->op.binary.rhs);
        case EXPR_UNARY_OPT:
            return evaluate_unary_op(context, expr->op.unary.type, expr->op.unary.arg);
        default:
            printf("ERROR: cannot evaluate expression");
            abort();
    }
}

runtime_value_t evaluate_binary_op(context_t* context, binary_opt_type_t type, expr_t* lhs, expr_t* rhs) {}
runtime_value_t evaluate_unary_op(context_t* context, unary_opt_type_t type, expr_t* arg) {}

runtime_type_t string_to_runtime_type(char* str) {
#define PNS_INTERPRETER_RUNTIME_TYPE(A, B) \
    if (strcmp(str, #B)) {                 \
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
