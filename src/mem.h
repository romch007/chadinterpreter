#ifndef CHAD_INTERPRETER_MEM_H
#define CHAD_INTERPRETER_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void* xmalloc(const size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: can't allocate %zu bytes\n", size);
        abort();
    }
    return ptr;
}

static inline void* xcalloc(const size_t count, const size_t size) {
    void* ptr = calloc(count, size);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: can't callocate %zu bytes\n", size);
        abort();
    }
    return ptr;
}

static inline void* xrealloc(void* original, const size_t size) {
    void* ptr = realloc(original, size);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: can't reallocate %zu bytes\n", size);
        abort();
    }
    return ptr;
}

static inline char* xstrdup(const char* str) {
    char* ptr = strdup(str);
    if (ptr == NULL) {
      fprintf(stderr, "ERROR: can't strdup string of length %zu\n", strlen(str));
      abort();
    }
    return ptr;
}

static inline char* extract_substr(const char* str, size_t start, size_t len) {
    char* substr = xmalloc(sizeof(char) * (len + 1));
    memcpy(substr, &str[start], len);
    substr[len] = '\0';
    return substr;
}

#endif
