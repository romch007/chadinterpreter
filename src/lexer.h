#ifndef CHAD_INTERPRETER_LEXER_H
#define CHAD_INTERPRETER_LEXER_H

#include <stdbool.h>
#include <stddef.h>

enum token_type {
#define CHAD_INTERPRETER_TOKEN(X) TOKEN_##X,
#include "tokens.h"
};

struct token {
    enum token_type type;
    int line_nb;
    union {
        char* str;
        long integer;
        bool boolean;
        double floating;
    } value;
};

struct token* tokenize(const char* input);

void print_tokens(struct token* tokens);
const char* token_type_to_string(enum token_type type);

#endif
