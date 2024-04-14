#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"
#include "mem.h"
#include "errors.h"
#include "stb_ds.h"
#include "stb_extra.h"

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

static bool is_blank(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

static bool is_valid_identifier(char c) {
    return c == '_' || isalpha(c);
}

struct token* tokenize(const char* input) {
    current_pos = 0;
    s_input = input;
    input_len = strlen(input);
    struct token* tokens = NULL;
    int current_line = 1;

    for (;;) {
        char c = peek(0);
        bool ignore = false;
        struct token token;

        if (c == '\0') {
            token.type = TOKEN_EOS;
            arrpush(tokens, token);
            break;
        }

        if (c == '\n') {
            current_line++;
            ignore = true;
        } else if (is_blank(c)) {
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
            } else if (peek(1) == '=') {
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
            } else if (peek(1) == '/') {
                // Line comment
                char next;
                do {
                    current_pos++;
                    next = peek(1);
                } while (next != '\0' && next != '\n');
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
                token.value.integer = atol(substr);
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
            } else if (strcmp(substr, "break") == 0) {
                token.type = TOKEN_BREAK;
            } else if (strcmp(substr, "continue") == 0) {
                token.type = TOKEN_CONTINUE;
            } else if (strcmp(substr, "return") == 0) {
                token.type = TOKEN_RETURN;
            } else if (strcmp(substr, "true") == 0) {
                token.type = TOKEN_BOOL_LITERAL;
                token.value.boolean = true;
            } else if (strcmp(substr, "false") == 0) {
                token.type = TOKEN_BOOL_LITERAL;
                token.value.boolean = false;
            } else if (strcmp(substr, "null") == 0) {
                token.type = TOKEN_NULL;
            } else if (strcmp(substr, "for") == 0) {
                token.type = TOKEN_FOR;
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
            panic("ERROR: invalid character %c, line %d", c, current_line);
        }

        token.line_nb = current_line;

        if (!ignore)
            arrpush(tokens, token);

        current_pos++;
    }

    return tokens;
}

const char* token_type_to_string(enum token_type type) {
    switch (type) {
#define CHAD_INTERPRETER_TOKEN(X) \
    case TOKEN_##X:               \
        return #X;
#include "tokens.h"
    }
    return NULL;
}

void print_tokens(struct token* tokens) {
    FOR_EACH(struct token, token, tokens) {
        printf("%s", token_type_to_string(token->type));
        if (token->type == TOKEN_INT_LITERAL) {
            printf(" - %ld\n", token->value.integer);
        } else if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_STR_LITERAL) {
            printf(" - %s\n", token->value.str);
        } else {
            printf("\n");
        }
    }
}
