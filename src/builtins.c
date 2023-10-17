#include "builtins.h"
#include <errno.h>

#ifdef HAVE_GETLINE
#include <stdio.h>
#else
#include "getline_impl.h"
#endif

builtin_fn_t is_builtin_fn(const char* fn_name) {
    if (strcmp(fn_name, "print") == 0) {
        return BUILTIN_FN_PRINT;
    } else if (strcmp(fn_name, "type") == 0) {
        return BUILTIN_FN_TYPE;
    } else if (strcmp(fn_name, "input") == 0) {
        return BUILTIN_FN_INPUT;
    } else if (strcmp(fn_name, "len") == 0) {
        return BUILTIN_FN_LEN;
    } else if (strcmp(fn_name, "at") == 0) {
        return BUILTIN_FN_AT;
    }

    return -1;
}

runtime_value_t execute_builtin(context_t* context, builtin_fn_t fn_type, cvector_vector_type(expr_t*) arguments) {
    switch (fn_type) {
        case BUILTIN_FN_PRINT: {
            expr_t** arg;
            for (arg = cvector_begin(arguments); arg != cvector_end(arguments); ++arg) {
                runtime_value_t value = evaluate_expr(context, *arg);
                print_value(&value);
                printf(" ");

                destroy_value(&value);
            }
            printf("\n");

            runtime_value_t return_value = { .type = RUNTIME_TYPE_NULL };

            return return_value;
        }
        case BUILTIN_FN_TYPE: {
            if (cvector_size(arguments) != 1) {
                printf("ERROR: 'type' function requires one argument\n");
                exit(EXIT_FAILURE);
            }

            expr_t* arg = arguments[0];
            runtime_value_t value = evaluate_expr(context, arg);

            runtime_value_t type_string = {
                    .type = RUNTIME_TYPE_STRING,
            };

            init_ref_counted(&type_string.value.string, copy_alloc(runtime_type_to_string(value.type)));

            destroy_value(&value);

            return type_string;
        }
        case BUILTIN_FN_INPUT: {
            runtime_value_t ps1_value;
            bool has_ps1 = false;

            if (cvector_size(arguments) > 1) {
                printf("ERROR: 'input' function requires zero or one argument(s)\n");
                exit(EXIT_FAILURE);
            }

            if (cvector_size(arguments) == 1) {
                has_ps1 = true;
                ps1_value = evaluate_expr(context, arguments[0]);

                if (ps1_value.type != RUNTIME_TYPE_STRING) {
                    printf("ERROR: 'input' can only accept str, not %s\n", runtime_type_to_string(ps1_value.type));
                    exit(EXIT_FAILURE);
                }
            }

            if (has_ps1)
                printf("%s", (char*)ps1_value.value.string.data);

            char* buffer = NULL;
            size_t len = 0;

            ssize_t read = getline(&buffer, &len, stdin);

            if (read < 0) {
                printf("ERROR: cannot get line: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            // Remove newline
            buffer[read - 1] = '\0';

            runtime_value_t result = {
                    .type = RUNTIME_TYPE_STRING,
            };

            init_ref_counted(&result.value.string, buffer);

            if (has_ps1)
                destroy_value(&ps1_value);

            return result;
        }
        case BUILTIN_FN_LEN: {
            if (cvector_size(arguments) != 1) {
                printf("ERROR: 'len' function requires one argument\n");
                exit(EXIT_FAILURE);
            }

            runtime_value_t input_value = evaluate_expr(context, arguments[0]);

            if (input_value.type != RUNTIME_TYPE_STRING) {
                printf("ERROR: cannot use 'len' on type %s\n", runtime_type_to_string(input_value.type));
                exit(EXIT_FAILURE);
            }

            int len = (int)strlen(input_value.value.string.data);

            runtime_value_t len_value = {
                    .type = RUNTIME_TYPE_INTEGER,
                    .value.integer = len,
            };

            destroy_value(&input_value);

            return len_value;
        }
        case BUILTIN_FN_AT: {
            if (cvector_size(arguments) != 2) {
                printf("ERROR: 'len' function requires two argument\n");
                exit(EXIT_FAILURE);
            }

            runtime_value_t target_value = evaluate_expr(context, arguments[0]);

            if (target_value.type != RUNTIME_TYPE_STRING) {
                printf("ERROR: cannot use 'at' on type %s\n", runtime_type_to_string(target_value.type));
                exit(EXIT_FAILURE);
            }

            runtime_value_t index_value = evaluate_expr(context, arguments[1]);

            if (index_value.type != RUNTIME_TYPE_INTEGER) {
                printf("ERROR: type %s cannot be use as an index\n", runtime_type_to_string(target_value.type));
                exit(EXIT_FAILURE);
            }

            int index = index_value.value.integer;
            char* origin = target_value.value.string.data;

            if (strlen(origin) <= index || index < 0) {
                printf("ERROR: index %d is out of bound\n", index);
                exit(EXIT_FAILURE);
            }

            char* chr = malloc(2);
            chr[0] = origin[index];
            chr[1] = '\0';

            runtime_value_t result = {
                    .type = RUNTIME_TYPE_STRING,
            };

            init_ref_counted(&result.value.string, chr);

            destroy_value(&target_value);
            destroy_value(&index_value);

            return result;
        }
        default:
            printf("ERROR: unknown builtin function\n");
            abort();
    }
}
