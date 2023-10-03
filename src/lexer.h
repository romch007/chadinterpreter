#ifndef PNS_INTERPRETER_LEXER_H
#define PNS_INTERPRETER_LEXER_H

#include "cvector.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
#define PNS_INTERPRETER_TOKEN(X) TOKEN_##X,
#include "tokens.h"
} token_type_t;

typedef struct {
    token_type_t type;
    union {
        char* str;
        int integer;
        bool boolean;
        double floating;
    } value;
} token_t;

cvector_vector_type(token_t) tokenize(const char* input);

void print_tokens(cvector_vector_type(token_t) tokens);
const char* token_type_to_string(token_type_t type);

void vector_token_deleter(void* element);

#endif
