#ifndef CHAD_INTERPRETER_STB_EXTRA_H
#define CHAD_INTERPRETER_STB_EXTRA_H

#define FOR_EACH(type, var, arr) for (type* var = arr; var <= arr + arrlen(arr) - 1; var++)

#define REVERSE_FOR_EACH(type, var, arr) for (type* var = arr + arrlen(arr); var-- != arr;)

#endif