#ifndef PNS_INTERPRETER_GC_H
#define PNS_INTERPRETER_GC_H

#include "mem.h"

typedef struct {
    int* reference_count;
    void* data;
} ref_counted_t;

void init_ref_counted(ref_counted_t* rc, void* data);

#endif