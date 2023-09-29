#ifndef PNS_INTERPRETER_MEM_H
#define PNS_INTERPRETER_MEM_H

#include <stdio.h>
#include <stdlib.h>

static void* xmalloc(const size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        printf("ERROR: can't allocate %zu bytes", size);
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

#endif
