#ifndef PNS_INTERPRETER_MEM_H
#define PNS_INTERPRETER_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* xmalloc(const size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("ERROR: can't allocate %zu bytes", size);
        abort();
    }
    return ptr;
}

static void* xcalloc(const size_t count, const size_t size) {
    void* ptr = calloc(count, size);
    if (ptr == NULL) {
        printf("ERROR: can't callocate %zu bytes", size);
        abort();
    }
    return ptr;
}

static void* xrealloc(void* original, const size_t size) {
    void* ptr = realloc(original, size);
    if (ptr == NULL) {
        printf("ERROR: can't reallocate %zu bytes", size);
        abort();
    }
    return ptr;
}

static char* copy_alloc(char* str) {
    char* allocated = xmalloc(sizeof(char) * strlen(str));
    strcpy(allocated, str);
    return allocated;
}

#endif
