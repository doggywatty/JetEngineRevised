// libs/glwt2/include/glwebtools/internal/glwebtools_memory.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_INTERNAL_GLWEBTOOLS_MEMORY_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_INTERNAL_GLWEBTOOLS_MEMORY_H

namespace glwebtools {
struct ExternalAlloc {  // sizeof 0x1
    static void* Malloc(unsigned);
    static void Free(void*);
    static void* Realloc(void*, unsigned);
    static char* Strdup(const char*);
    static void* Calloc(unsigned, unsigned);
};
}

#endif
