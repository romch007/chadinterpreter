#ifndef PNS_INTERPRETER_LEXER_H
#define PNS_INTERPRETER_LEXER_H

#include "cvector.h"
#include <stddef.h>

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_EQUAL,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_INT_LITERAL,
    TOKEN_STR_LITERAL,
    TOKEN_SEMICOLON,
    TOKEN_EOS
} token_type_t;

typedef struct {
    token_type_t type;
    union {
        char* str;
        int integer;
    } value;
} token_t;

cvector_vector_type(token_t) tokenize(char* input);

void print_tokens(cvector_vector_type(token_t) tokens);
char* token_type_to_string(token_type_t type);

#endif
