#include "builtins.h"
#include "errors.h"
#include <errno.h>

#ifdef HAVE_GETLINE
#include <stdio.h>
#else
#include "getline_impl.h"
#endif

builtin_fn_t is_builtin_fn(const char* fn_name) {
#define CHAD_INTERPRETER_BUILTIN_FN(A, B) \
    if (strcmp(fn_name, #B) == 0) {       \
        return BUILTIN_FN_##A;            \
    }
#include "builtin_fns.h"
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
                panic("ERROR: 'type' function requires one argument\n");
            }

            expr_t* arg = arguments[0];
            runtime_value_t value = evaluate_expr(context, arg);

            runtime_value_t type_string = {
                    .type = RUNTIME_TYPE_STRING,
            };

            init_ref_counted(&type_string.value.string, xstrdup(runtime_type_to_string(value.type)));

            destroy_value(&value);

            return type_string;
        }
        case BUILTIN_FN_INPUT: {
            runtime_value_t ps1_value;
            bool has_ps1 = false;

            if (cvector_size(arguments) > 1) {
                panic("ERROR: 'input' function requires zero or one argument(s)\n");
            }

            if (cvector_size(arguments) == 1) {
                has_ps1 = true;
                ps1_value = evaluate_expr(context, arguments[0]);

                if (ps1_value.type != RUNTIME_TYPE_STRING) {
                    panic("ERROR: 'input' can only accept str, not %s\n", runtime_type_to_string(ps1_value.type));
                }
            }

            if (has_ps1)
                printf("%s", (char*)ps1_value.value.string.data);

            char* buffer = NULL;
            size_t len = 0;

            ssize_t read = getline(&buffer, &len, stdin);

            if (read < 0) {
                panic("ERROR: cannot get line: %s\n", strerror(errno));
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
                panic("ERROR: 'len' function requires one argument\n");
            }

            runtime_value_t input_value = evaluate_expr(context, arguments[0]);

            if (input_value.type != RUNTIME_TYPE_STRING) {
                panic("ERROR: cannot use 'len' on type %s\n", runtime_type_to_string(input_value.type));
            }

            long len = (long)strlen(input_value.value.string.data);

            runtime_value_t len_value = {
                    .type = RUNTIME_TYPE_INTEGER,
                    .value.integer = len,
            };

            destroy_value(&input_value);

            return len_value;
        }
        case BUILTIN_FN_AT: {
            if (cvector_size(arguments) != 2) {
                panic("ERROR: 'len' function requires two argument\n");
            }

            runtime_value_t target_value = evaluate_expr(context, arguments[0]);

            if (target_value.type != RUNTIME_TYPE_STRING) {
                panic("ERROR: cannot use 'at' on type %s\n", runtime_type_to_string(target_value.type));
            }

            runtime_value_t index_value = evaluate_expr(context, arguments[1]);

            if (index_value.type != RUNTIME_TYPE_INTEGER) {
                panic("ERROR: type %s cannot be use as an index\n", runtime_type_to_string(target_value.type));
            }

            long index = index_value.value.integer;
            char* origin = target_value.value.string.data;

            if (strlen(origin) <= index || index < 0) {
                panic("ERROR: index %ld is out of bound\n", index);
            }

            char* chr = xmalloc(2);
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
            fprintf(stderr, "ERROR: unknown builtin function\n");
            abort();
    }
}
