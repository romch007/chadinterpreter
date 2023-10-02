#include "mem.h"


extern inline void* xmalloc(const size_t size);
extern inline void* xcalloc(const size_t count, const size_t size);
extern inline void* xrealloc(void* original, const size_t size);
extern inline char* copy_alloc(const char* str);
extern inline char* extract_substr(const char* str, size_t start, size_t len);
