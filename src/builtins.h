#ifndef CHAD_INTERPRETER_BUILTINS_H
#define CHAD_INTERPRETER_BUILTINS_H

#include "interpreter.h"
#include "ast.h"

typedef enum {
    BUILTIN_FN_PRINT,
    BUILTIN_FN_TYPE,
    BUILTIN_FN_INPUT,
    BUILTIN_FN_LEN,
    BUILTIN_FN_AT,
} builtin_fn;

builtin_fn is_builtin_fn(const char* fn_name);
runtime_value_t execute_builtin(context_t* context, builtin_fn fn_type, cvector_vector_type(expr_t*) arguments);

#endif