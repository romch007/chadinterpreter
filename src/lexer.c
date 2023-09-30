#include "lexer.h"
#include "mem.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static size_t current_pos = 0;
static char* input = NULL;
static size_t input_len = 0;


static char peek(int advance) {
    if (current_pos + advance < input_len && input[current_pos + advance] != '\0') {
        return input[current_pos + advance];
    } else {
        return '\0';
    }
}

static int str_to_int(const char* str, size_t len) {
    size_t i;
    int ret = 0;
    for (i = 0; i < len; ++i) {
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}

static void vector_token_deleter(void* element) {
    token_t* token = (token_t*) element;
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STR_LITERAL) {
        free(token->value.str);
    }
}

static bool is_blank(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

static bool is_valid_identifier(char c) {
    return c == '_' || isalpha(c);
}

cvector_vector_type(token_t) tokenize(char* value) {
    current_pos = 0;
    input = value;
    input_len = strlen(value);
    cvector_vector_type(token_t) tokens = NULL;
    cvector_init(tokens, 0, &vector_token_deleter);

    for (;;) {
        char c = peek(0);
        bool ignore = false;
        token_t token;

        if (c == '\0') {
            token.type = TOKEN_EOS;
            cvector_push_back(tokens, token);
            break;
        }

        if (is_blank(c)) {
            ignore = true;
        } else if (c == '+') {
            token.type = TOKEN_PLUS;
        } else if (c == '-') {
            token.type = TOKEN_MINUS;
        } else if (c == '*') {
            token.type = TOKEN_MUL;
        } else if (c == '/') {
            token.type = TOKEN_DIV;
        } else if (c == '(') {
            token.type = TOKEN_OPEN_PAREN;
        } else if (c == ')') {
            token.type = TOKEN_CLOSE_PAREN;
        } else if (c == '=') {
            token.type = TOKEN_EQUAL;
        } else if (c == ';') {
            token.type = TOKEN_SEMICOLON;
        } else if (c == ':') {
            token.type = TOKEN_COLON;
        } else if (isdigit(c)) {
            size_t start = current_pos;
            char next;

            for (;;) {
                next = peek(1);
                if (!isdigit(next)) {
                    break;
                }

                current_pos++;
            }

            const char* repr = &input[start];
            token.type = TOKEN_INT_LITERAL;
            token.value.integer = str_to_int(repr, (current_pos - start) + 1);
        } else if (is_valid_identifier(c)) {
            size_t start = current_pos;

            while (isalpha(peek(1))) current_pos++;

            size_t len = (current_pos - start) + 1;

            if (strncmp(&input[start], "let", len) == 0) {
                token.type = TOKEN_LET;
            } else if (strncmp(&input[start], "const", len) == 0) {
                token.type = TOKEN_CONST;
            } else {
                token.type = TOKEN_IDENTIFIER;
                token.value.str = xmalloc(sizeof(char) * len);
                strncpy(token.value.str, &input[start], len);
            }
        } else if (c == '\'') {
            size_t start = current_pos;

            while (peek(1) != '\'') current_pos++;
            start++;

            size_t len = (current_pos - start) + 1;
            token.type = TOKEN_STR_LITERAL;
            token.value.str = xmalloc(sizeof(char) * len);
            strncpy(token.value.str, &input[start], len);
            current_pos++;
        } else {
            printf("ERROR: invalid character %c", c);
            abort();
        }

        if (!ignore)
            cvector_push_back(tokens, token);

        current_pos++;
    }

    return tokens;
}

char* token_type_to_string(token_type_t type) {
    switch (type) {
#define PNS_INTERPRETER_TOKEN(X) \
    case TOKEN_##X:              \
        return #X;
#include "tokens.h"
    }
}

void print_tokens(cvector_vector_type(token_t) tokens) {
    for (token_t* token = cvector_begin(tokens); token != cvector_end(tokens); ++token) {
        printf("%s", token_type_to_string(token->type));
        if (token->type == TOKEN_INT_LITERAL) {
            printf(" - %d\n", token->value.integer);
        } else if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STR_LITERAL) {
            printf(" - %s\n", token->value.str);
        } else {
            printf("\n");
        }
    }
}
