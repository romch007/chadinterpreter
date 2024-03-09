#ifndef CHAD_INTERPRETER_BUILTINS_H
#define CHAD_INTERPRETER_BUILTINS_H

#include "interpreter.h"
#include "ast.h"

typedef enum {
#define CHAD_INTERPRETER_BUILTIN_FN(A, B) BUILTIN_FN_##A,
#include "builtin_fns.h"
} builtin_fn_t;

builtin_fn_t is_builtin_fn(const char* fn_name);
struct runtime_value execute_builtin(struct context* context, builtin_fn_t fn_type, cvector_vector_type(struct expr*) arguments);

#endif