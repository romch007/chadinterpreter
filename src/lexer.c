#include "lexer.h"
#include "mem.h"
#include <ctype.h>
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

vc_vector* tokenize(char* value) {
    current_pos = 0;
    input = value;
    input_len = strlen(value);
    vc_vector* tokens = vc_vector_create(0, sizeof(token_t), &vector_token_deleter);

    for (;;) {
        char c = peek(0);
        bool ignore = false;
        token_t token;

        if (c == '\0') {
            token.type = TOKEN_EOS;
            vc_vector_push_back(tokens, &token);
            break;
        }

        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                ignore = true;
                break;// Ignore blanks
            case '+':
                token.type = TOKEN_PLUS;
                break;
            case '-':
                token.type = TOKEN_MINUS;
                break;
            case '*':
                token.type = TOKEN_MUL;
                break;
            case '/':
                token.type = TOKEN_DIV;
                break;
            case '(':
                token.type = TOKEN_OPEN_PAREN;
                break;
            case ')':
                token.type = TOKEN_CLOSE_PAREN;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
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
                break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'w':
            case 'x':
            case 'y':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y': {
                size_t start = current_pos;

                while (isalpha(peek(1))) current_pos++;

                size_t len = (current_pos - start) + 1;
                token.type = TOKEN_IDENTIFIER;
                token.value.str = xmalloc(sizeof(char) * len);
                strncpy(token.value.str, &input[start], len);
                break;
            }
            case '\'': {
                size_t start = current_pos;

                while (peek(1) != '\'') current_pos++;
                start++;

                size_t len = (current_pos - start) + 1;
                token.type = TOKEN_STR_LITERAL;
                token.value.str = xmalloc(sizeof(char) * len);
                strncpy(token.value.str, &input[start], len);
                current_pos++;
                break;
            }
            default:
                printf("ERROR: invalid character %c", c);
                return NULL;
        }

        if (!ignore)
            vc_vector_push_back(tokens, &token);

        current_pos++;
    }

    return tokens;
}

static char* token_type_to_string(token_type_t type) {
    switch (type) {
        case TOKEN_IDENTIFIER:
            return "IDENTIFIER";
        case TOKEN_PLUS:
            return "PLUS";
        case TOKEN_MINUS:
            return "MINUS";
        case TOKEN_MUL:
            return "MUL";
        case TOKEN_DIV:
            return "DIV";
        case TOKEN_INT_LITERAL:
            return "INT_LITERAL";
        case TOKEN_STR_LITERAL:
            return "STR_LITERAL";
        case TOKEN_EOS:
            return "EOS";
        case TOKEN_OPEN_PAREN:
            return "OPEN_PAREN";
        case TOKEN_CLOSE_PAREN:
            return "CLOSE_PAREN";
        default:
            return "<UNKNOWN>";
    }
}

void print_tokens(vc_vector* tokens) {
    for (void* i = vc_vector_begin(tokens);
         i != vc_vector_end(tokens);
         i = vc_vector_next(tokens, i)) {
        token_t* token = (token_t*) i;
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
