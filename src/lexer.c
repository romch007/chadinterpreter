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

void vector_token_deleter(void* element) {
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
            if (peek(1) == '>') {
                current_pos++;
                token.type = TOKEN_ARROW;
            } else {
                token.type = TOKEN_MINUS;
            }
        } else if (c == '*') {
            token.type = TOKEN_MUL;
        } else if (c == '/') {
            token.type = TOKEN_DIV;
        } else if (c == '!') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_NOT_EQUAL;
            } else {
                token.type = TOKEN_NOT;
            }
        } else if (c == '&') {
            if (peek(1) == '&') {
                current_pos++;
                token.type = TOKEN_AND;
            }
        } else if (c == '|') {
            if (peek(1) == '|') {
                current_pos++;
                token.type = TOKEN_OR;
            }
        } else if (c == '(') {
            token.type = TOKEN_OPEN_PAREN;
        } else if (c == ')') {
            token.type = TOKEN_CLOSE_PAREN;
        } else if (c == '{') {
            token.type = TOKEN_OPEN_BRACE;
        } else if (c == '}') {
            token.type = TOKEN_CLOSE_BRACE;
        } else if (c == '=') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_DOUBLE_EQUAL;
            } else {
                token.type = TOKEN_EQUAL;
            }
        } else if (c == ';') {
            token.type = TOKEN_SEMICOLON;
        } else if (c == ':') {
            token.type = TOKEN_COLON;
        } else if (c == ',') {
            token.type = TOKEN_COMMA;
        } else if (c == '<') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_LESS_EQUAL;
            } else {
                token.type = TOKEN_LESS;
            }
        } else if (c == '>') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_GREATER_EQUAL;
            } else {
                token.type = TOKEN_GREATER;
            }
        } else if (isdigit(c)) {
            bool floating_point = false;
            size_t start = current_pos;
            char next;

            for (;;) {
                next = peek(1);
                if (!isdigit(next)) {
                    if (next != '.') break;

                    if (floating_point) break;

                    floating_point = true;
                }

                current_pos++;
            }

            size_t len = (current_pos - start) + 1;

            char* substr = xmalloc(sizeof(char) * (len + 1));
            memcpy(substr, &input[start], len);
            substr[len] = '\0';

            if (floating_point) {
                token.type = TOKEN_FLOAT_LITERAL;
                token.value.floating = atof(substr);
            } else {
                token.type = TOKEN_INT_LITERAL;
                token.value.integer = atoi(substr);
            }

            free(substr);
        } else if (is_valid_identifier(c)) {
            size_t start = current_pos;

            while (is_valid_identifier(peek(1))) current_pos++;

            size_t len = (current_pos - start) + 1;

            // TODO: fix these strncmp (to pass through valgrind)
            if (strncmp(&input[start], "if", len) == 0) {
                token.type = TOKEN_IF;
            } else if (strncmp(&input[start], "else", len) == 0) {
                token.type = TOKEN_ELSE;
            } else if (strncmp(&input[start], "while", len) == 0) {
                token.type = TOKEN_WHILE;
            } else if (strncmp(&input[start], "fn", len) == 0) {
                token.type = TOKEN_FN;
            } else if (strncmp(&input[start], "let", len) == 0) {
                token.type = TOKEN_LET;
            } else if (strncmp(&input[start], "const", len) == 0) {
                token.type = TOKEN_CONST;
            } else if (strncmp(&input[start], "true", len) == 0) {
                token.type = TOKEN_BOOL_LITERAL;
                token.value.boolean = true;
            } else if (strncmp(&input[start], "false", len) == 0) {
                token.type = TOKEN_BOOL_LITERAL;
                token.value.boolean = false;
            } else {
                token.type = TOKEN_IDENTIFIER;
                token.value.str = xmalloc(sizeof(char) * (len + 1));
                strncpy(token.value.str, &input[start], len);
                token.value.str[len] = '\0';
            }
        } else if (c == '\'') {
            size_t start = current_pos;

            while (peek(1) != '\'') current_pos++;
            start++;

            size_t len = (current_pos - start) + 1;
            token.type = TOKEN_STR_LITERAL;
            token.value.str = xmalloc(sizeof(char) * (len + 1));
            strncpy(token.value.str, &input[start], len);
            token.value.str[len] = '\0';
            current_pos++;
        } else {
            printf("ERROR: invalid character %c", c);
            exit(1);
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
