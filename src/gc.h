#ifndef CHAD_INTERPRETER_GC_H
#define CHAD_INTERPRETER_GC_H

#include "mem.h"

typedef struct {
    int* reference_count;
    void* data;
} ref_counted_t;

inline void init_ref_counted(ref_counted_t* rc, void* data) {
    rc->data = data;
    rc->reference_count = xmalloc(sizeof(int));
    *rc->reference_count = 0;
}

#endif