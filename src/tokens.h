#if !defined(CHAD_INTERPRETER_TOKEN)
#error You must define CHAD_INTERPRETER_TOKEN before including this file
#endif

#ifndef CHAD_INTERPRETER_TOKEN_LAST
#define CHAD_INTERPRETER_TOKEN_LAST(X) CHAD_INTERPRETER_TOKEN(X)
#endif

CHAD_INTERPRETER_TOKEN(IDENTIFIER)
CHAD_INTERPRETER_TOKEN(LET)
CHAD_INTERPRETER_TOKEN(CONST)
CHAD_INTERPRETER_TOKEN(IF)
CHAD_INTERPRETER_TOKEN(ELSE)
CHAD_INTERPRETER_TOKEN(WHILE)
CHAD_INTERPRETER_TOKEN(FN)
CHAD_INTERPRETER_TOKEN(BREAK)
CHAD_INTERPRETER_TOKEN(CONTINUE)
CHAD_INTERPRETER_TOKEN(PLUS)
CHAD_INTERPRETER_TOKEN(MINUS)
CHAD_INTERPRETER_TOKEN(MUL)
CHAD_INTERPRETER_TOKEN(DIV)
CHAD_INTERPRETER_TOKEN(MODULO)
CHAD_INTERPRETER_TOKEN(PLUS_EQUAL)
CHAD_INTERPRETER_TOKEN(MINUS_EQUAL)
CHAD_INTERPRETER_TOKEN(MUL_EQUAL)
CHAD_INTERPRETER_TOKEN(DIV_EQUAL)
CHAD_INTERPRETER_TOKEN(MODULO_EQUAL)
CHAD_INTERPRETER_TOKEN(OR)
CHAD_INTERPRETER_TOKEN(AND)
CHAD_INTERPRETER_TOKEN(NOT)
CHAD_INTERPRETER_TOKEN(EQUAL)
CHAD_INTERPRETER_TOKEN(DOUBLE_EQUAL)
CHAD_INTERPRETER_TOKEN(NOT_EQUAL)
CHAD_INTERPRETER_TOKEN(GREATER)
CHAD_INTERPRETER_TOKEN(GREATER_EQUAL)
CHAD_INTERPRETER_TOKEN(LESS)
CHAD_INTERPRETER_TOKEN(LESS_EQUAL)
CHAD_INTERPRETER_TOKEN(OPEN_PAREN)
CHAD_INTERPRETER_TOKEN(CLOSE_PAREN)
CHAD_INTERPRETER_TOKEN(OPEN_BRACE)
CHAD_INTERPRETER_TOKEN(CLOSE_BRACE)
CHAD_INTERPRETER_TOKEN(BOOL_LITERAL)
CHAD_INTERPRETER_TOKEN(INT_LITERAL)
CHAD_INTERPRETER_TOKEN(FLOAT_LITERAL)
CHAD_INTERPRETER_TOKEN(STR_LITERAL)
CHAD_INTERPRETER_TOKEN(SEMICOLON)
CHAD_INTERPRETER_TOKEN(COLON)
CHAD_INTERPRETER_TOKEN(COMMA)
CHAD_INTERPRETER_TOKEN(ARROW)
CHAD_INTERPRETER_TOKEN(EOS)

#undef CHAD_INTERPRETER_TOKEN
#undef CHAD_INTERPRETER_TOKEN_LAST