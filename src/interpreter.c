#include "interpreter.h"
#include "builtins.h"
#include "errors.h"
#include "mem.h"
#include "stb_ds.h"
#include "stb_extra.h"

static int variable_compare(const void* a, const void* b, void* udata) {
    const struct runtime_variable* va = a;
    const struct runtime_variable* vb = b;

    return strcmp(va->name, vb->name);
}

static uint64_t variable_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const struct runtime_variable* v = item;
    return hashmap_xxhash3(v->name, strlen(v->name), seed0, seed1);
}

static int function_compare(const void* a, const void* b, void* udata) {
    const struct statement** fa = (const struct statement**) a;
    const struct statement** fb = (const struct statement**) b;

    return strcmp((*fa)->op.function_declaration.fn_name, (*fb)->op.function_declaration.fn_name);
}

static uint64_t function_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const struct statement** f = (const struct statement**) item;
    return hashmap_xxhash3((*f)->op.function_declaration.fn_name, strlen((*f)->op.function_declaration.fn_name), seed0, seed1);
}

void init_context(struct context* context) {
    context->frames = NULL;
    context->should_break_loop = false;
    context->should_continue_loop = false;
    context->should_return_fn = false;
    context->recursion_depth = 0;
}

static struct stack_frame* get_current_stack_frame(struct context* context) {
    return context->frames + arrlen(context->frames) - 1;
}

void dump_context(struct context* context) {
    printf("--- variables ---\n");
    FOR_EACH(struct stack_frame, it, context->frames) {
        dump_stack_frame(it);
    }
}


void dump_stack_frame(struct stack_frame* frame) {
    size_t iter = 0;
    void* item;
    while (hashmap_iter(frame->variables, &iter, &item)) {
        const struct runtime_variable* variable = item;
        printf("%s: %s = ", variable->name, runtime_type_to_string(variable->content.type));
        print_value(&variable->content);
    }
}

void print_value(const struct runtime_value* value) {
    switch (value->type) {
        case RUNTIME_TYPE_STRING:
            printf("%s", value->value.string.data ? (char*) value->value.string.data : "(empty)");
            break;
        case RUNTIME_TYPE_INTEGER:
            printf("%ld", value->value.integer);
            break;
        case RUNTIME_TYPE_FLOAT:
            printf("%f", value->value.floating);
            break;
        case RUNTIME_TYPE_BOOLEAN:
            printf("%s", value->value.boolean ? "true" : "false");
            break;
        case RUNTIME_TYPE_NULL:
            printf("(null)");
            break;
    }
}

void destroy_value(const struct runtime_value* value) {
    // Destroy the content if no reference are held anymore
    if (value->type == RUNTIME_TYPE_STRING && *value->value.string.reference_count <= 0) {
        free(value->value.string.data);
        free(value->value.string.reference_count);
    }
}

void destroy_context(struct context* context) {
    for (int i = 0; i < arrlen(context->frames); i++) {
        pop_stack_frame(context);
    }
    arrfree(context->frames);
}

void push_stack_frame(struct context* context) {
    struct stack_frame frame = {
            .variables = hashmap_new(sizeof(struct runtime_variable), 0, 0, 0, variable_hash, variable_compare, NULL, NULL),
            .functions = hashmap_new(sizeof(struct statement*), 0, 0, 0, function_hash, function_compare, NULL, NULL),
    };

    arrpush(context->frames, frame);
}

void pop_stack_frame(struct context* context) {
    struct stack_frame* frame = get_current_stack_frame(context);
    // Free variables
    size_t iter = 0;
    void* item;
    while (hashmap_iter(frame->variables, &iter, &item)) {
        const struct runtime_variable* variable = item;
        free(variable->name);

        // If the variable content is reference-counted, decrement the reference count
        if (variable->content.type == RUNTIME_TYPE_STRING)
            (*variable->content.value.string.reference_count)--;

        destroy_value(&variable->content);
    }
    hashmap_free(frame->variables);
    // Functions are freed during AST destruction
    hashmap_free(frame->functions);
    arrpop(context->frames);
}

void execute_statement(struct context* context, struct statement* statement) {
    switch (statement->type) {
        case STATEMENT_BLOCK: {
            FOR_EACH(struct statement*, it, statement->op.block.statements) {
                execute_statement(context, *it);

                if (context->should_break_loop || context->should_continue_loop || context->should_return_fn)
                    break;
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
        case STATEMENT_NAKED_FN_CALL: {
            struct runtime_value discarded_return_value = evaluate_expr(context, statement->op.naked_fn_call.function_call);

            destroy_value(&discarded_return_value);
            break;
        }
        case STATEMENT_VARIABLE_ASSIGN: {
            execute_variable_assignment(context, statement);
            break;
        }
        case STATEMENT_IF_CONDITION: {
            struct runtime_value condition = evaluate_expr(context, statement->op.if_condition.condition);

            if (condition.type != RUNTIME_TYPE_BOOLEAN) {
                panic("ERROR: found a value of type %s in a if condition\n", runtime_type_to_string(condition.type));
            }

            struct statement* body = statement->op.if_condition.body;
            struct statement* body_else = statement->op.if_condition.body_else;

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
            struct runtime_value condition = evaluate_expr(context, statement->op.while_loop.condition);

            if (condition.type != RUNTIME_TYPE_BOOLEAN) {
                panic("ERROR: found a value of type %s in a while condition\n", runtime_type_to_string(condition.type));
            }

            push_stack_frame(context);
            while (condition.value.boolean) {
                execute_statement(context, statement->op.while_loop.body);

                if (context->should_break_loop) {
                    context->should_break_loop = false;
                    break;
                } else if (context->should_continue_loop) {
                    context->should_continue_loop = false;
                }

                condition = evaluate_expr(context, statement->op.while_loop.condition);
            }
            pop_stack_frame(context);
            break;
        }
        case STATEMENT_FOR_LOOP: {
            push_stack_frame(context);
            for (execute_statement(context, statement->op.for_loop.initializer);
                evaluate_expr(context, statement->op.for_loop.condition).value.boolean;
                execute_statement(context, statement->op.for_loop.increment)) {

                execute_statement(context, statement->op.for_loop.body);

                if (context->should_break_loop) {
                    context->should_break_loop = false;
                    break;
                } else if (context->should_continue_loop) {
                    context->should_continue_loop = false;
                }
            }

            pop_stack_frame(context);
            break;
        }
        case STATEMENT_BREAK:
            context->should_break_loop = true;
            break;
        case STATEMENT_CONTINUE:
            context->should_continue_loop = true;
            break;
        case STATEMENT_RETURN:
            if (statement->op.return_statement.value != NULL) {
                struct runtime_value return_value = evaluate_expr(context, statement->op.return_statement.value);

                context->has_return_value = true;
                context->return_value = return_value;
            }
            context->should_return_fn = true;
            break;
        default:
            fprintf(stderr, "ERROR: cannot execute statement\n");
            abort();
    }
}

void execute_variable_assignment(struct context* context, struct statement* statement) {
    char* variable_name = statement->op.variable_assignment.variable_name;

    int stack_index;

    const struct runtime_variable* old_variable = get_variable(context, variable_name, &stack_index);

    if (old_variable == NULL) {
        panic("ERROR: cannot find variable '%s'\n", variable_name);
    }

    if (old_variable->is_constant) {
        panic("ERROR: variable '%s' is constant\n", variable_name);
    }

    struct runtime_value new_content = evaluate_expr(context, statement->op.variable_assignment.value);

    if (old_variable->content.type != new_content.type) {
        panic("ERROR: cannot assign value of type %s to variable '%s' of type %s\n", runtime_type_to_string(new_content.type), variable_name, runtime_type_to_string(old_variable->content.type));
    }

    // Increment reference count if value content is reference-counted
    if (new_content.type == RUNTIME_TYPE_STRING) {
        (*new_content.value.string.reference_count)++;
    }

    struct runtime_variable variable = {
            .name = old_variable->name,
            .content = new_content,
            .is_constant = false,
    };

    hashmap_set(context->frames[stack_index].variables, &variable);
}

void execute_variable_declaration(struct context* context, struct statement* statement) {
    char* variable_name = statement->op.variable_declaration.variable_name;

    // Check if this declaration is shadowing a constant variable
    const struct runtime_variable* old_variable = get_variable(context, variable_name, NULL);

    if (old_variable != NULL && old_variable->is_constant == true) {
        panic("ERROR: declaration of '%s' is shadowing a constant variable\n", variable_name);
    }

    struct runtime_variable variable = {
            .name = xstrdup(variable_name),
            .is_constant = statement->op.variable_declaration.is_constant,
    };

    if (statement->op.variable_declaration.value == NULL) {
        variable.content.type = RUNTIME_TYPE_NULL;
    } else {
        struct runtime_value value = evaluate_expr(context, statement->op.variable_declaration.value);

        variable.content = value;
    }

    // Increment reference count if value content is reference-counted
    if (variable.content.type == RUNTIME_TYPE_STRING) {
        (*variable.content.value.string.reference_count)++;
    }

    hashmap_set(get_current_stack_frame(context)->variables, &variable);
}

const struct runtime_variable* get_variable(struct context* context, const char* variable_name, int* stack_index) {
    int i = arrlen(context->frames) - 1;

    REVERSE_FOR_EACH(struct stack_frame, it, context->frames) {
        const struct runtime_variable* variable = (const struct runtime_variable*) hashmap_get(it->variables, &(struct runtime_variable){.name = (char*) variable_name});

        if (variable != NULL) {
            if (stack_index != NULL) *stack_index = i;
            return variable;
        }

        i--;
    }

    return NULL;
}

const struct statement* get_function(struct context* context, const char* fn_name, int* stack_index) {
    int i = arrlen(context->frames) - 1;

    REVERSE_FOR_EACH(struct stack_frame, it, context->frames) {
        struct statement search_term = {.type = STATEMENT_FUNCTION_DECL, .op.function_declaration.fn_name = (char*) fn_name};
        struct statement* search_term_ptr = &search_term;
        const struct statement** fn = (const struct statement**) hashmap_get(it->functions, &search_term_ptr);

        if (fn != NULL) {
            if (stack_index != NULL) *stack_index = i;
            return *fn;
        }

        i--;
    }

    return NULL;
}

struct runtime_value evaluate_expr(struct context* context, struct expr* expr) {
    switch (expr->type) {
        case EXPR_BOOL_LITERAL: {
            struct runtime_value value = {
                    .type = RUNTIME_TYPE_BOOLEAN,
                    .value.boolean = expr->op.bool_literal};

            return value;
        }
        case EXPR_INT_LITERAL: {
            struct runtime_value value = {
                    .type = RUNTIME_TYPE_INTEGER,
                    .value.integer = expr->op.integer_literal};

            return value;
        }
        case EXPR_FLOAT_LITERAL: {
            struct runtime_value value = {
                    .type = RUNTIME_TYPE_FLOAT,
                    .value.floating = expr->op.float_literal};

            return value;
        }
        case EXPR_STRING_LITERAL: {
            struct runtime_value value = {
                    .type = RUNTIME_TYPE_STRING,
            };

            init_ref_counted(&value.value.string, xstrdup(expr->op.string_literal));

            return value;
        }
        case EXPR_NULL: {
            struct runtime_value value = {
                    .type = RUNTIME_TYPE_NULL,
            };

            return value;
        }
        case EXPR_VARIABLE_USE: {
            char* variable_name = expr->op.variable_use.name;

            const struct runtime_variable* variable = get_variable(context, variable_name, NULL);

            if (variable == NULL) {
                panic("ERROR: cannot find variable '%s'\n", variable_name);
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
            fprintf(stderr, "ERROR: cannot evaluate expression\n");
            abort();
    }
}

struct runtime_value evaluate_binary_op(struct context* context, enum binary_op_type op_type, struct expr* lhs, struct expr* rhs) {
    struct runtime_value lhs_value = evaluate_expr(context, lhs);
    struct runtime_value rhs_value = evaluate_expr(context, rhs);

    if (lhs_value.type != rhs_value.type) {
        panic("ERROR: type mismatch between %s and %s\n", runtime_type_to_string(lhs_value.type), runtime_type_to_string(rhs_value.type));
    }

    enum runtime_type value_type = lhs_value.type;

    struct runtime_value result_value;

    if (is_arithmetic_binary_op(op_type)) {
        if (op_type == BINARY_OP_ADD && value_type == RUNTIME_TYPE_STRING) {
            // String concat
            size_t new_size = strlen(lhs_value.value.string.data) + strlen(rhs_value.value.string.data) + 1;
            char* buffer = xmalloc(new_size);
            memset(buffer, 0, new_size);
            strcat(buffer, lhs_value.value.string.data);
            strcat(buffer, rhs_value.value.string.data);

            result_value.type = RUNTIME_TYPE_STRING;

            init_ref_counted(&result_value.value.string, buffer);
        } else if (value_type == RUNTIME_TYPE_INTEGER) {
            // Arithmetic operations with integers
            result_value.type = RUNTIME_TYPE_INTEGER;

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
                        panic("ERROR: cannot divide by zero\n");
                    }
                    result_value.value.integer = lhs_value.value.integer / rhs_value.value.integer;
                    break;
                case BINARY_OP_MODULO:
                    result_value.value.integer = lhs_value.value.integer % rhs_value.value.integer;
                default:
                    break;
            }
        } else if (value_type == RUNTIME_TYPE_FLOAT) {
            if (op_type == BINARY_OP_MODULO) {
                panic("ERROR: cannot use modulo on float values\n");
            }
            // Arithmetic operations with floats
            result_value.type = RUNTIME_TYPE_FLOAT;

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
                        panic("ERROR: cannot divide by zero\n");
                    }
                    result_value.value.floating = lhs_value.value.floating / rhs_value.value.floating;
                    break;
                default:
                    break;
            }
        } else {
            panic("ERROR: cannot use arithmetic operator on type %s\n", runtime_type_to_string(value_type));
        }
    } else if (is_logical_binary_op(op_type)) {
        if (value_type != RUNTIME_TYPE_BOOLEAN) {
            panic("ERROR: cannot use logical operator on type %s\n", runtime_type_to_string(value_type));
        }

        result_value.type = RUNTIME_TYPE_BOOLEAN;

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
    } else if (is_comparison_binary_op(op_type)) {
        result_value.type = RUNTIME_TYPE_BOOLEAN;

        if (value_type == RUNTIME_TYPE_STRING) {
            // String comparison
            int cmp_result = strcmp(lhs_value.value.string.data, rhs_value.value.string.data);

            result_value.type = RUNTIME_TYPE_BOOLEAN;
            result_value.value.integer = cmp_result == 0;
        } else if (value_type == RUNTIME_TYPE_INTEGER) {
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
        } else if (value_type == RUNTIME_TYPE_FLOAT) {
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
        } else {
            panic("ERROR: cannot use comparison operator on type %s\n", runtime_type_to_string(value_type));
        }
    } else {
        fprintf(stderr, "ERROR: unknown binary operator\n");
        abort();
    }

    destroy_value(&lhs_value);
    destroy_value(&rhs_value);

    return result_value;
}

struct runtime_value evaluate_unary_op(struct context* context, enum unary_op_type op_type, struct expr* arg) {
    struct runtime_value arg_value = evaluate_expr(context, arg);

    struct runtime_value result_value;

    if (op_type == UNARY_OP_NOT) {
        if (arg_value.type != RUNTIME_TYPE_BOOLEAN) {
            panic("ERROR: cannot use logical operation on type %s\n", runtime_type_to_string(arg_value.type));
        }

        result_value.type = RUNTIME_TYPE_BOOLEAN;

        result_value.value.boolean = !arg_value.value.boolean;
    } else if (op_type == UNARY_OP_NEG) {
        if (arg_value.type != RUNTIME_TYPE_INTEGER && arg_value.type != RUNTIME_TYPE_FLOAT) {
            panic("ERROR: cannot use logical operation on type %s\n", runtime_type_to_string(arg_value.type));
        }

        result_value.type = arg_value.type;

        switch (arg_value.type) {
            case RUNTIME_TYPE_FLOAT:
                result_value.value.floating = -arg_value.value.floating;
                break;
            case RUNTIME_TYPE_INTEGER:
                result_value.value.integer = -arg_value.value.integer;
                break;
            default:
                break;
        }

    } else {
        fprintf(stderr, "ERROR: unknown unary operator\n");
        abort();
    }

    destroy_value(&arg_value);

    return result_value;
}

struct runtime_value evaluate_function_call(struct context* context, const char* fn_name, struct expr** arguments) {
    struct runtime_value return_value = {
            .type = RUNTIME_TYPE_NULL};

    builtin_fn_t fn_type;
    if ((fn_type = is_builtin_fn(fn_name)) != -1) {
        return execute_builtin(context, fn_type, arguments);
    }

    const struct statement* fn = get_function(context, fn_name, NULL);

    if (fn == NULL) {
        panic("ERROR: cannot find function %s\n", fn_name);
    }

    size_t fn_decl_argument_size = arrlen(fn->op.function_declaration.arguments);
    size_t fn_call_argument_size = arrlen(arguments);

    if (fn_decl_argument_size != fn_call_argument_size) {
        panic("ERROR: '%s' expects %zu arguments, but %zu were given\n", fn_name, fn_decl_argument_size, fn_call_argument_size);
    }

    push_stack_frame(context);

    // Evaluate arguments
    struct runtime_value* evaluated_arguments = NULL;
    arrsetcap(evaluated_arguments, fn_call_argument_size);

    FOR_EACH(struct expr*, arg_value, arguments)  {
        struct runtime_value value = evaluate_expr(context, *arg_value);
        arrpush(evaluated_arguments, value);
    }

    // Inject arguments values into stack frame
    for (size_t i = 0; i < arrlen(evaluated_arguments); i++) {
        struct runtime_value value = evaluated_arguments[i];
        if (value.type == RUNTIME_TYPE_STRING) {
            (*value.value.string.reference_count)++;
        }
        struct runtime_variable variable = {
                .name = xstrdup(fn->op.function_declaration.arguments[i]),
                .is_constant = false,
                .content = value,
        };
        hashmap_set(get_current_stack_frame(context)->variables, &variable);
    }

    arrfree(evaluated_arguments);

    context->recursion_depth++;

    if (context->recursion_depth >= MAX_RECURSION_DEPTH) {
        panic("ERROR: max recursion depth exceeded\n");
    }

    execute_statement(context, fn->op.function_declaration.body);
    context->recursion_depth--;

    if (context->should_return_fn) {
        context->should_return_fn = false;

        if (context->has_return_value) {
            return_value = context->return_value;
            context->has_return_value = false;
        }
    }

    pop_stack_frame(context);
    return return_value;
}

enum runtime_type string_to_runtime_type(const char* str) {
#define CHAD_INTERPRETER_RUNTIME_TYPE(A, B) \
    if (strcmp(str, #B) == 0) {             \
        return RUNTIME_TYPE_##A;            \
    }
#include "runtime_types.h"
    return -1;
}

const char* runtime_type_to_string(enum runtime_type type) {
    switch (type) {
#define CHAD_INTERPRETER_RUNTIME_TYPE(A, B) \
    case RUNTIME_TYPE_##A:                  \
        return #B;
#include "runtime_types.h"
    }

    return NULL;
}
