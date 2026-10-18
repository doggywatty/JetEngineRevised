// core/jet/include/mem/memorymanager.h
#ifndef CORE_JET_INCLUDE_MEM_MEMORYMANAGER_H
#define CORE_JET_INCLUDE_MEM_MEMORYMANAGER_H

#include <cstddef>
#include <vector>
#include "core/pstring.h"

namespace jet {
namespace mem {

// Allocation wrappers. Z = zero filled, NZ = not; _S realloc variants take the old size.
// With tracking on (SetTrackingEnabled), live blocks are counted per tag; tags are a per-thread stack.
void* Malloc_Z_S(size_t size);
void* Malloc_NZ_S(size_t size);
void* Realloc_Z_S(void* p, size_t oldSize, size_t size);
void* Realloc_Z(void* p, size_t size);
void* Realloc_NZ_S(void* p, size_t oldSize, size_t size);
void* Realloc_NZ(void* p, size_t size);
void Free_S(void* p);

void SetTrackingEnabled(bool on);
bool IsTrackingEnabled();
size_t GetAllocatedTotal();
unsigned GetAllocationCount();
std::vector<String> GetTagNames();
size_t GetAllocatedPerTag(const String& tag);
void SetAllocationTag(const char* tag);
void PushAllocationTag();
void PopAllocationTag();
void Reserve(size_t size);  // counts memory allocated outside these wrappers
void Release(size_t size);

// Scoped tag: PushAllocationTag + SetAllocationTag, popped on destruction.
struct Tag {
    explicit Tag(const char* name)
    {
        PushAllocationTag();
        SetAllocationTag(name);
    }
    ~Tag() { PopAllocationTag(); }
};

template <class T>
struct malloc_allocator {
    typedef T value_type;
    malloc_allocator() {}
    template <class U>
    malloc_allocator(const malloc_allocator<U>&) {}
    T* allocate(size_t n) { return static_cast<T*>(Malloc_NZ_S(n * sizeof(T))); }
    void deallocate(T* p, size_t) { Free_S(p); }
    template <class U>
    bool operator==(const malloc_allocator<U>&) const { return true; }
    template <class U>
    bool operator!=(const malloc_allocator<U>&) const { return false; }
};

}
}

#endif
