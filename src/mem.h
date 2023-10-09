#ifndef CHAD_INTERPRETER_MEM_H
#define CHAD_INTERPRETER_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif

inline void* xmalloc(const size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("ERROR: can't allocate %zu bytes", size);
        abort();
    }
    return ptr;
}

inline void* xcalloc(const size_t count, const size_t size) {
    void* ptr = calloc(count, size);
    if (ptr == NULL) {
        printf("ERROR: can't callocate %zu bytes", size);
        abort();
    }
    return ptr;
}

inline void* xrealloc(void* original, const size_t size) {
    void* ptr = realloc(original, size);
    if (ptr == NULL) {
        printf("ERROR: can't reallocate %zu bytes", size);
        abort();
    }
    return ptr;
}

inline char* copy_alloc(const char* str) {
    size_t len = strlen(str);
    char* allocated = xmalloc(sizeof(char) * (len + 1));
    memcpy(allocated, str, len + 1);
    return allocated;
}

inline char* extract_substr(const char* str, size_t start, size_t len) {
    char* substr = xmalloc(sizeof(char) * (len + 1));
    memcpy(substr, &str[start], len);
    substr[len] = '\0';
    return substr;
}

#endif