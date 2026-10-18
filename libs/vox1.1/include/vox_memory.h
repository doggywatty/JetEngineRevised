// libs/vox1.1/include/vox_memory.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_MEMORY_H
#define LIBS_VOX1_1_INCLUDE_VOX_MEMORY_H

#include <cstddef>
#include <cstdlib>
#include <new>
#include "vox_types.h"

// @0x63b890: VoxAlloc(size, hint, file, function, line); @0x63b888 / 0x63b88c / 0x63b8a0 are its short forms; @0x63b8a4 VoxFree
void *VoxAlloc(unsigned size, vox::VoxMemHint hint, const char *file, const char *function, int line);
void *VoxAlloc(unsigned size, const char *file, const char *function, int line);
void *VoxAlloc(unsigned size);
void *VoxAlloc(unsigned size, vox::VoxMemHint hint);
void VoxFree(void *p);

namespace vox {
// the allocator the engine's containers use (std::vector<T, vox::SAllocator<T, 0> > in the .so)
template <class T, int Hint = 0>
class SAllocator {
public:
    typedef T value_type;
    SAllocator() {}
    template <class U> SAllocator(const SAllocator<U, Hint>&) {}
    T *allocate(size_t n) { return static_cast<T*>(::VoxAlloc((unsigned)(n * sizeof(T)))); }
    void deallocate(T *p, size_t) { ::VoxFree(p); }
    template <class U> struct rebind { typedef SAllocator<U, Hint> other; };
    bool operator==(const SAllocator&) const { return true; }
    bool operator!=(const SAllocator&) const { return false; }
};
}  // namespace vox

#endif
