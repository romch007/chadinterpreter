#include "gc.h"

void init_ref_counted(ref_counted_t* rc, void* data) {
    rc->data = data;
    rc->reference_count = xmalloc(sizeof(int));
    *rc->reference_count = 0;
}
