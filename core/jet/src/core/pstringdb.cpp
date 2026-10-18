// core/jet/src/core/pstringdb.cpp
#include "core/pstringdb.h"
#include <cstring>

namespace jet {

static const char kEmpty[] = "";

void String::Cell::Reset()
{
    length = 0;
    hash = 0;
    key = kEmpty;
    string = kEmpty;
    next = nullptr;
    nextSibling = nullptr;
    counterData = 0;
    counter = &counterData;
}

StringDBMap::StringDBMap() : mCells(0x10000, nullptr), mGarbageCollectCounter(0)
{
    mCellPools.push_back(new CellPool);
}

StringDBMap::~StringDBMap()
{
    for (String::Cell* c : mCells)
        for (; c; c = c->next)
            FreeStringFamily(*c);
    for (CellPool* p : mCellPools)
        delete p;
}

String::Cell& StringDBMap::FindOrAdd(unsigned hash, int length, const char* s)
{
    if (mGarbageCollectCounter > 999) {
        mGarbageCollectCounter = 0;
        GarbageCollect();
    }
    String::Cell*& bucket = mCells[(hash & 0xffff) ^ (hash >> 16)];
    String::Cell* prev = nullptr;
    for (String::Cell* c = bucket; c; prev = c, c = c->next) {
        if (c->length != length)
            continue;
        if (memcmp(c->string, s, length) == 0)
            return *c;
        int i = 0;
        while (i < length && detail::tolowerascii(c->string[i]) == detail::tolowerascii(s[i]))
            ++i;
        if (i != length)
            continue;
        String::Cell* sib = c;
        for (; sib->nextSibling; sib = sib->nextSibling)
            if (memcmp(sib->nextSibling->string, s, length) == 0)
                return *sib->nextSibling;
        String::Cell* n = GetNewCell();
        CopyString(*n, length, s);
        n->hash = c->hash;
        n->key = c->key;
        n->counter = &c->counterData;
        sib->nextSibling = n;
        return *n;
    }
    String::Cell* n = GetNewCell();
    CopyString(*n, length, s);
    n->hash = hash;
    n->key = n->string;
    (prev ? prev->next : bucket) = n;
    return *n;
}

String::Cell* StringDBMap::GetNewCell()
{
    ++mGarbageCollectCounter;
    if (!mFreeCells.empty()) {
        String::Cell* c = mFreeCells.back();
        mFreeCells.pop_back();
        c->Reset();
        return c;
    }
    if (mCellPools.back()->used >= CellPool::kCells)
        mCellPools.push_back(new CellPool);
    CellPool* p = mCellPools.back();
    return &p->cells[p->used++];
}

void StringDBMap::CopyString(String::Cell& c, int length, const char* s)
{
    c.length = length;
    char* d = length < 31 ? c.stringStorage : new char[length + 1];
    memcpy(d, s, length);
    d[length] = 0;
    c.string = d;
}

void StringDBMap::FreeStringFamily(String::Cell& head)
{
    for (String::Cell* c = &head; c; c = c->nextSibling) {
        c->counter = nullptr;
        if (c->string != c->stringStorage && c->string != kEmpty && c->string)
            delete[] c->string;
        c->key = nullptr;
        c->string = nullptr;
    }
}

unsigned StringDBMap::GarbageCollect()
{
    unsigned freed = 0;
    for (String::Cell*& bucket : mCells) {
        String::Cell* prev = nullptr;
        for (String::Cell* c = bucket; c;) {
            String::Cell* next = c->next;
            if (c->counter->load() == 0) {
                FreeStringFamily(*c);
                ++freed;
                (prev ? prev->next : bucket) = next;
            } else {
                prev = c;
            }
            c = next;
        }
    }
    mFreeCells.clear();
    for (CellPool* p : mCellPools)
        for (unsigned i = 0; i < p->used; ++i)
            if (!p->cells[i].counter)
                mFreeCells.push_back(&p->cells[i]);
    return freed;
}

StringDB::StringDB() : mKeysCount(0), mInstancesCount(0)
{
    mStringMap.FindOrAdd(0, 0, kEmpty);
    ++mKeysCount;
    ++mInstancesCount;
}

StringDB& StringDB::GetInstance()
{
    static StringDB db;
    return db;
}

// MurmurHash2 over the ASCII-lowercased bytes (seed 0xff4f5).
static unsigned HashNoCase(const char* s, unsigned len)
{
    const unsigned m = 0x5bd1e995;
    unsigned h = len ^ 0xff4f5;
    int n = len;
    for (; n > 3; n -= 4, s += 4) {
        unsigned k;
        memcpy(&k, s, 4);
        k = m * (k | 0x20202020);
        h = (m * (k ^ (k >> 24))) ^ (m * h);
    }
    switch (n) {
    case 3: h ^= (u8(s[2]) | 0x20) << 16;
    case 2: h ^= (u8(s[1]) | 0x20) << 8;
    case 1: h = m * (h ^ (u8(s[0]) | 0x20));
    }
    h = m * (h ^ (h >> 13));
    return h ^ (h >> 15);
}

void StringDB::InternalizeString(const char* b, const char* e, String::StringData& out)
{
    unsigned len = unsigned(e - b);
    unsigned h = HashNoCase(b, len);
    std::lock_guard<std::mutex> lock(mMutex);
    String::Cell* c = &mStringMap.FindOrAdd(h, len, b);
    c->counter->fetch_add(1);
    String::StringData d(c);
    out = d;
}

}
