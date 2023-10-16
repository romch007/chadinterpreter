#include "builtins.h"
#include <errno.h>

#ifdef HAVE_GETLINE
#include <stdio.h>
#else
#error TODO: implements getline
#endif

builtin_fn is_builtin_fn(const char* fn_name) {
    if (strcmp(fn_name, "print") == 0) {
        return BUILTIN_FN_PRINT;
    } else if (strcmp(fn_name, "type") == 0) {
        return BUILTIN_FN_TYPE;
    } else if (strcmp(fn_name, "input") == 0) {
        return BUILTIN_FN_INPUT;
    }

    return -1;
}

runtime_value_t execute_builtin(context_t* context, builtin_fn fn_type, cvector_vector_type(expr_t*) arguments) {
    switch (fn_type) {
        case BUILTIN_FN_PRINT: {
            expr_t** arg;
            for (arg = cvector_begin(arguments); arg != cvector_end(arguments); ++arg) {
                runtime_value_t value = evaluate_expr(context, *arg);
                print_value(&value);

                destroy_value(&value);
            }

            runtime_value_t return_value = { .type = RUNTIME_TYPE_NULL };

            return return_value;
        }
        case BUILTIN_FN_TYPE: {
            if (cvector_size(arguments) != 1) {
                printf("ERROR: 'type' function require one argument\n");
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
            if (cvector_size(arguments) != 0) {
                printf("ERROR: 'input' function don't require arguments\n");
                exit(EXIT_FAILURE);
            }

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

            return result;
        }
        default:
            printf("ERROR: unknown builtin function\n");
            abort();
    }
}
