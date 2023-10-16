#include "interpreter.h"
#include "mem.h"

void print_value(const runtime_value_t* value);
static int variable_compare(const void* a, const void* b, void* udata) {
    const runtime_variable_t* va = a;
    const runtime_variable_t* vb = b;

    return strcmp(va->name, vb->name);
}

static uint64_t variable_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const runtime_variable_t* v = item;
    return hashmap_xxhash3(v->name, strlen(v->name), seed0, seed1);
}

static int function_compare(const void* a, const void* b, void* udata) {
    const statement_t** fa = (const statement_t**) a;
    const statement_t** fb = (const statement_t**) b;

    return strcmp((*fa)->op.function_declaration.fn_name, (*fb)->op.function_declaration.fn_name);
}

static uint64_t function_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const statement_t** f = (const statement_t**) item;
    return hashmap_xxhash3((*f)->op.function_declaration.fn_name, strlen((*f)->op.function_declaration.fn_name), seed0, seed1);
}

context_t* create_context() {
    context_t* context = xmalloc(sizeof(context_t));
    context->frames = NULL;
    return context;
}

static stack_frame_t* get_current_stack_frame(context_t* context) {
    return cvector_end(context->frames) - 1;
}

void dump_context(context_t* context) {
    printf("--- variables ---\n");
    stack_frame_t* it;
    for (it = cvector_begin(context->frames); it != cvector_end(context->frames); it++) {
        dump_stack_frame(it);
    }
}


void dump_stack_frame(stack_frame_t* frame) {
    size_t iter = 0;
    void* item;
    while (hashmap_iter(frame->variables, &iter, &item)) {
        const runtime_variable_t* variable = item;
        printf("%s: %s = ", variable->name, runtime_type_to_string(variable->content.type));
        print_value(&variable->content);
    }
}

void print_value(const runtime_value_t* value) {
    switch (value->type) {
        case RUNTIME_TYPE_STRING:
            printf("%s\n", value->value.string.data ? (char*) value->value.string.data : "(empty)");
            break;
        case RUNTIME_TYPE_INTEGER:
            printf("%d\n", value->value.integer);
            break;
        case RUNTIME_TYPE_FLOAT:
            printf("%f\n", value->value.floating);
            break;
        case RUNTIME_TYPE_BOOLEAN:
            printf("%s\n", value->value.boolean ? "true" : "false");
            break;
        case RUNTIME_TYPE_NULL:
            printf("(null)\n");
            break;
    }
}

void destroy_value(const runtime_value_t* value) {
    if (value->type == RUNTIME_TYPE_STRING) {
        // If the variable content is reference-counted, decrement the reference count
        (*value->value.string.reference_count)--;

        // Destroy the content if no reference are held anymore
        if (*value->value.string.reference_count < 0) {
            free(value->value.string.data);
            free(value->value.string.reference_count);
        }
    }
}

void destroy_context(context_t* context) {
    for (int i = 0; i < cvector_size(context->frames); i++) {
        pop_stack_frame(context);
    }
    cvector_free(context->frames);
    free(context);
}

void push_stack_frame(context_t* context) {
    stack_frame_t frame = {
            .variables = hashmap_new(sizeof(runtime_variable_t), 0, 0, 0, variable_hash, variable_compare, NULL, NULL),
            .functions = hashmap_new(sizeof(statement_t*), 0, 0, 0, function_hash, function_compare, NULL, NULL),
    };

    cvector_push_back(context->frames, frame);
}

void pop_stack_frame(context_t* context) {
    stack_frame_t* frame = get_current_stack_frame(context);
    // Free variables
    size_t iter = 0;
    void* item;
    while (hashmap_iter(frame->variables, &iter, &item)) {
        const runtime_variable_t* variable = item;
        free(variable->name);
        destroy_value(&variable->content);
    }
    hashmap_free(frame->variables);
    // Functions are freed during AST destruction
    hashmap_free(frame->functions);
    cvector_pop_back(context->frames);
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
            execute_variable_declaration(context, statement);
            break;
        }
        case STATEMENT_FUNCTION_DECL:
            hashmap_set(get_current_stack_frame(context)->functions, &statement);
            break;
        case STATEMENT_NAKED_FN_CALL:
            evaluate_expr(context, statement->op.naked_fn_call.function_call);
            break;
        case STATEMENT_VARIABLE_ASSIGN: {
            execute_variable_assignment(context, statement);
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

            push_stack_frame(context);
            if (condition.value.boolean) {
                execute_statement(context, body);
            } else if (body_else != NULL) {
                execute_statement(context, body_else);
            }
            pop_stack_frame(context);
            break;
        }
        case STATEMENT_WHILE_LOOP: {
            runtime_value_t condition = evaluate_expr(context, statement->op.while_loop.condition);

            if (condition.type != RUNTIME_TYPE_BOOLEAN) {
                printf("ERROR: found a value of type %s in a while condition\n", runtime_type_to_string(condition.type));
                exit(EXIT_FAILURE);
            }

            push_stack_frame(context);
            while (condition.value.boolean) {
                execute_statement(context, statement->op.while_loop.body);

                condition = evaluate_expr(context, statement->op.while_loop.condition);
            }
            pop_stack_frame(context);
            break;
        }
        default:
            printf("ERROR: cannot execute statement\n");
            abort();
    }
}

void execute_variable_assignment(context_t* context, statement_t* statement) {
    char* variable_name = statement->op.variable_assignment.variable_name;

    int stack_index;

    const runtime_variable_t* old_variable = get_variable(context, variable_name, &stack_index);

    if (old_variable == NULL) {
        printf("ERROR: cannot find variable '%s'\n", variable_name);
        exit(EXIT_FAILURE);
    }

    if (old_variable->is_constant) {
        printf("ERROR: variable '%s' is constant\n", variable_name);
        exit(EXIT_FAILURE);
    }

    runtime_value_t new_content = evaluate_expr(context, statement->op.variable_assignment.value);

    if (old_variable->content.type != new_content.type) {
        printf("ERROR: cannot assign value of type %s to variable '%s' of type %s\n", runtime_type_to_string(new_content.type), variable_name, runtime_type_to_string(old_variable->content.type));
        exit(EXIT_FAILURE);
    }

    // Increment reference count if value content is reference-counted
    if (new_content.type == RUNTIME_TYPE_STRING) {
        (*new_content.value.string.reference_count)++;
    }

    runtime_variable_t variable = {
            .name = old_variable->name,
            .content = new_content,
            .is_constant = false,
    };

    hashmap_set(context->frames[stack_index].variables, &variable);
}

void execute_variable_declaration(context_t* context, statement_t* statement) {
    char* variable_name = statement->op.variable_declaration.variable_name;

    // Check if this declaration is shadowing a constant variable
    const runtime_variable_t* old_variable = get_variable(context, variable_name, NULL);

    if (old_variable != NULL && old_variable->is_constant == true) {
        printf("ERROR: declaration of '%s' is shadowing a constant variable\n", variable_name);
        exit(EXIT_FAILURE);
    }

    runtime_variable_t variable = {
            .name = copy_alloc(variable_name),
            .is_constant = statement->op.variable_declaration.is_constant,
    };

    if (statement->op.variable_declaration.value == NULL) {
        variable.content.type = RUNTIME_TYPE_NULL;
    } else {
        runtime_value_t default_value = evaluate_expr(context, statement->op.variable_declaration.value);

        variable.content = default_value;
    }

    // Increment reference count if value content is reference-counted
    if (variable.content.type == RUNTIME_TYPE_STRING) {
        (*variable.content.value.string.reference_count)++;
    }

    hashmap_set(get_current_stack_frame(context)->variables, &variable);
}

const runtime_variable_t* get_variable(context_t* context, const char* variable_name, int* stack_index) {
    stack_frame_t* it;
    int i = cvector_size(context->frames) - 1;
    for (it = cvector_end(context->frames); it-- != cvector_begin(context->frames);) {
        const runtime_variable_t* variable = (const runtime_variable_t*) hashmap_get(it->variables, &(runtime_variable_t){.name = (char*) variable_name});

        if (variable != NULL) {
            if (stack_index != NULL) *stack_index = i;
            return variable;
        }

        i--;
    }

    return NULL;
}

const statement_t* get_function(context_t* context, const char* fn_name, int* stack_index) {
    stack_frame_t* it;
    int i = cvector_size(context->frames) - 1;
    for (it = cvector_end(context->frames); it-- != cvector_begin(context->frames);) {
        statement_t search_term = { .type = STATEMENT_FUNCTION_DECL, .op.function_declaration.fn_name = (char*)fn_name };
        statement_t* search_term_ptr = &search_term;
        const statement_t** fn = (const statement_t **) hashmap_get(it->functions, &search_term_ptr);

        if (fn != NULL) {
            if (stack_index != NULL) *stack_index = i;
            return *fn;
        }

        i--;
    }

    return NULL;
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
            };

            init_ref_counted(&value.value.string, copy_alloc(expr->op.string_literal));

            return value;
        }
        case EXPR_VARIABLE_USE: {
            char* variable_name = expr->op.variable_use.name;

            const runtime_variable_t* variable = get_variable(context, variable_name, NULL);

            if (variable == NULL) {
                printf("ERROR: cannot find variable '%s'\n", variable_name);
                exit(EXIT_FAILURE);
            }

            return variable->content;
        }
        case EXPR_FUNCTION_CALL: {
            return evaluate_function_call(context, expr->op.function_call.name, expr->op.function_call.arguments);
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
                case BINARY_OP_MODULO:
                    result_value.value.integer = lhs_value.value.integer % rhs_value.value.integer;
                default:
                    break;
            }

            return result_value;
        } else {
            if (op_type == BINARY_OP_MODULO) {
                printf("ERROR: cannot use modulo on float values\n");
                exit(EXIT_FAILURE);
            }
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

runtime_value_t evaluate_function_call(context_t* context, const char* fn_name, cvector_vector_type(expr_t*) arguments) {
    runtime_value_t fake_return_value = {
        .type = RUNTIME_TYPE_NULL
    };

    // TODO: proper built-in functions
    if (strcmp(fn_name, "print") == 0) {
        expr_t **arg;
        for (arg = cvector_begin(arguments); arg != cvector_end(arguments); ++arg) {
            runtime_value_t value = evaluate_expr(context, *arg);
            print_value(&value);
            destroy_value(&value);
        }

        return fake_return_value;
    }

    const statement_t* fn = get_function(context, fn_name, NULL);

    if (fn == NULL) {
        printf("ERROR: cannot find function %s\n", fn_name);
        exit(EXIT_FAILURE);
    }
    push_stack_frame(context);

    // Inject arguments into stack frame
    expr_t** arg_value;
    int arg_index = 0;
    for (arg_value = cvector_begin(arguments); arg_value != cvector_end(arguments); ++arg_value) {
        runtime_value_t value = evaluate_expr(context, *arg_value);
        if (value.type == RUNTIME_TYPE_STRING) {
            (*value.value.string.reference_count)++;
        }
        runtime_variable_t variable = {
                .name = copy_alloc(fn->op.function_declaration.arguments[arg_index]),
                .is_constant = false,
                .content = value,
        };
        hashmap_set(get_current_stack_frame(context)->variables, &variable);
        arg_index++;
    }

    execute_statement(context, fn->op.function_declaration.body);
    pop_stack_frame(context);

    return fake_return_value;
}

runtime_type_t string_to_runtime_type(const char* str) {
#define CHAD_INTERPRETER_RUNTIME_TYPE(A, B) \
    if (strcmp(str, #B) == 0) {             \
        return RUNTIME_TYPE_##A;            \
    }
#include "runtime_types.h"
    return -1;
}

const char* runtime_type_to_string(runtime_type_t type) {
    switch (type) {
#define CHAD_INTERPRETER_RUNTIME_TYPE(A, B) \
    case RUNTIME_TYPE_##A:                  \
        return #B;
#include "runtime_types.h"
    }

    return NULL;
}
