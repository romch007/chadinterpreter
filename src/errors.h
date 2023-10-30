#ifndef CHAD_INTERPRETER_ERRORS_H
#define CHAD_INTERPRETER_ERRORS_H

#include <stdio.h>
#include <stdlib.h>

#define panic(...) fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE)

#endif
