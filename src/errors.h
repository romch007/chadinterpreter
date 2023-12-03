#ifndef CHAD_INTERPRETER_ERRORS_H
#define CHAD_INTERPRETER_ERRORS_H

#include <stdio.h>
#include <stdlib.h>

#define panic(...) do { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); } while (0)

#endif
