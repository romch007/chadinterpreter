#ifndef CHAD_INTERPRETER_BUILTINS_H
#define CHAD_INTERPRETER_BUILTINS_H

#include "interpreter.h"
#include "ast.h"

typedef enum {
#define CHAD_INTERPRETER_BUILTIN_FN(A, B) BUILTIN_FN_##A,
#include "builtin_fns.h"
} builtin_fn_t;

builtin_fn_t is_builtin_fn(const char* fn_name);
runtime_value_t execute_builtin(context_t* context, builtin_fn_t fn_type, cvector_vector_type(expr_t*) arguments);

#endif