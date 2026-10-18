// core/jet/src/mem/memorymanager.cpp
#include "mem/memorymanager.h"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <string>

namespace jet {
namespace mem {

namespace {

enum { kMaxTags = 256, kMaxDepth = 999 };

struct BlockInfo {
    size_t size;
    int tag;
};

// Plain malloc containers so the tracker never recurses into itself.
typedef std::basic_string<char, std::char_traits<char>, malloc_allocator<char>> TagName;

struct State {
    std::recursive_mutex mutex;
    std::map<void*, BlockInfo, std::less<void*>, malloc_allocator<std::pair<void* const, BlockInfo>>> blocks;
    std::map<TagName, int, std::less<TagName>, malloc_allocator<std::pair<const TagName, int>>> tags;
    size_t perTag[kMaxTags] = {};
    int tagCount = 0;
};

std::atomic<bool> s_tracking(false);
std::atomic<size_t> s_total(0);
std::atomic<unsigned> s_count(0);

State& S()
{
    static State* s = new (malloc(sizeof(State))) State;
    return *s;
}

thread_local int t_depth = 0;
thread_local int t_stack[kMaxDepth + 1] = {};

int TagId(const char* name)
{
    State& s = S();
    TagName n(name);
    auto it = s.tags.find(n);
    if (it != s.tags.end())
        return it->second;
    int id = s.tagCount < kMaxTags - 1 ? ++s.tagCount : kMaxTags - 1;
    s.tags[n] = id;
    return id;
}

void Track(void* p, size_t size)
{
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    int tag = t_stack[t_depth];
    s.blocks[p] = BlockInfo{size, tag};
    s.perTag[tag] += size;
    s_total += size;
}

void Untrack(void* p)
{
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    auto it = s.blocks.find(p);
    if (it == s.blocks.end())
        return;
    s.perTag[it->second.tag] -= it->second.size;
    s_total -= it->second.size;
    s.blocks.erase(it);
}

void* Done(void* p, size_t size)
{
    if (s_tracking && p)
        Track(p, size);
    ++s_count;
    return p;
}

}

void* Malloc_Z_S(size_t size) { return Done(calloc(size, 1), size); }

void* Malloc_NZ_S(size_t size) { return Done(malloc(size), size); }

void* Realloc_Z_S(void* p, size_t oldSize, size_t size)
{
    if (s_tracking && p)
        Untrack(p);
    void* r = realloc(p, size);
    if (r && oldSize < size)
        memset(static_cast<char*>(r) + oldSize, 0, size - oldSize);
    return Done(r, size);
}

void* Realloc_Z(void* p, size_t size) { return Realloc_Z_S(p, 0, size); }

void* Realloc_NZ_S(void* p, size_t, size_t size)
{
    if (s_tracking && p)
        Untrack(p);
    return Done(realloc(p, size), size);
}

void* Realloc_NZ(void* p, size_t size) { return Realloc_NZ_S(p, 0, size); }

void Free_S(void* p)
{
    if (!p)
        return;
    if (s_tracking)
        Untrack(p);
    free(p);
}

void SetTrackingEnabled(bool on) { s_tracking = on; }
bool IsTrackingEnabled() { return s_tracking; }
size_t GetAllocatedTotal() { return s_total; }
unsigned GetAllocationCount() { return s_count; }

std::vector<String> GetTagNames()
{
    std::vector<String> r;
    if (!s_tracking)
        return r;
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    for (const auto& kv : s.tags)
        r.push_back(String(kv.first.c_str()));
    return r;
}

size_t GetAllocatedPerTag(const String& tag)
{
    if (!s_tracking)
        return 0;
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    return s.perTag[TagId(tag.c_str())];
}

void SetAllocationTag(const char* tag)
{
    if (!s_tracking)
        return;
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    t_stack[t_depth] = TagId(tag);
}

void PushAllocationTag()
{
    if (!s_tracking || t_depth >= kMaxDepth)
        return;
    ++t_depth;
    t_stack[t_depth] = t_stack[t_depth - 1];
}

void PopAllocationTag()
{
    if (s_tracking && t_depth > 0)
        --t_depth;
}

void Reserve(size_t size)
{
    if (!s_tracking)
        return;
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    s_total += size;
    s.perTag[t_stack[t_depth]] += size;
}

void Release(size_t size)
{
    if (!s_tracking)
        return;
    State& s = S();
    std::lock_guard<std::recursive_mutex> l(s.mutex);
    s_total -= size < s_total ? size : s_total.load();
    size_t& t = s.perTag[t_stack[t_depth]];
    t -= size < t ? size : t;
}

}
}
