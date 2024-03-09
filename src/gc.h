#ifndef CHAD_INTERPRETER_GC_H
#define CHAD_INTERPRETER_GC_H

#include "mem.h"

struct ref_counted {
    int* reference_count;
    void* data;
};

inline void init_ref_counted(struct ref_counted* rc, void* data) {
    rc->data = data;
    rc->reference_count = xmalloc(sizeof(int));
    *rc->reference_count = 0;
}

#endif