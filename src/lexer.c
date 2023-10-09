#include "lexer.h"
#include "mem.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static size_t current_pos = 0;
static const char* s_input = NULL;
static size_t input_len = 0;

static char peek(int advance) {
    if (current_pos + advance < input_len && s_input[current_pos + advance] != '\0') {
        return s_input[current_pos + advance];
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

cvector_vector_type(token_t) tokenize(const char* input) {
    current_pos = 0;
    s_input = input;
    input_len = strlen(input);
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
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_PLUS_EQUAL;
            } else {
                token.type = TOKEN_PLUS;
            }
        } else if (c == '-') {
            if (peek(1) == '>') {
                current_pos++;
                token.type = TOKEN_ARROW;
            }  else if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_MINUS_EQUAL;
            } else {
                token.type = TOKEN_MINUS;
            }
        } else if (c == '*') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_MUL_EQUAL;
            } else {
                token.type = TOKEN_MUL;
            }
        } else if (c == '/') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_DIV_EQUAL;
            } else {
                token.type = TOKEN_DIV;
            }
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
        } else if (c == '%') {
            if (peek(1) == '=') {
                current_pos++;
                token.type = TOKEN_MODULO_EQUAL;
            } else {
                token.type = TOKEN_MODULO;
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

            char* substr = extract_substr(input, start, len);

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
            char* substr = extract_substr(input, start, len);
            bool keep = false;

            if (strcmp(substr, "if") == 0) {
                token.type = TOKEN_IF;
            } else if (strcmp(substr, "else") == 0) {
                token.type = TOKEN_ELSE;
            } else if (strcmp(substr, "while") == 0) {
                token.type = TOKEN_WHILE;
            } else if (strcmp(substr, "fn") == 0) {
                token.type = TOKEN_FN;
            } else if (strcmp(substr, "let") == 0) {
                token.type = TOKEN_LET;
            } else if (strcmp(substr, "const") == 0) {
                token.type = TOKEN_CONST;
            } else if (strcmp(substr, "true") == 0) {
                token.type = TOKEN_BOOL_LITERAL;
                token.value.boolean = true;
            } else if (strcmp(substr, "false") == 0) {
                token.type = TOKEN_BOOL_LITERAL;
                token.value.boolean = false;
            } else {
                token.type = TOKEN_IDENTIFIER;
                token.value.str = substr;
                keep = true;
            }

            if (!keep) free(substr);
        } else if (c == '"') {
            size_t start = current_pos;

            while (peek(1) != '"') current_pos++;
            start++;

            size_t len = (current_pos - start) + 1;
            token.type = TOKEN_STR_LITERAL;
            token.value.str = extract_substr(input, start, len);
            current_pos++;
        } else {
            printf("ERROR: invalid character %c", c);
            exit(EXIT_FAILURE);
        }

        if (!ignore)
            cvector_push_back(tokens, token);

        current_pos++;
    }

    return tokens;
}

const char* token_type_to_string(token_type_t type) {
    switch (type) {
#define CHAD_INTERPRETER_TOKEN(X) \
    case TOKEN_##X:              \
        return #X;
#include "tokens.h"
    }
    return NULL;
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
