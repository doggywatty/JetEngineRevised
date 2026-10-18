// core/jet/include/core/pstringdb.h
#ifndef CORE_JET_INCLUDE_CORE_PSTRINGDB_H
#define CORE_JET_INCLUDE_CORE_PSTRINGDB_H

#include <mutex>
#include <vector>
#include "core/pstring.h"

namespace jet {

class StringDBMap {
public:
    StringDBMap();
    ~StringDBMap();
    String::Cell& FindOrAdd(unsigned hash, int length, const char* s);

private:
    struct CellPool {
        enum { kCells = 64 };
        String::Cell cells[kCells];
        unsigned used = 0;
    };

    void CopyString(String::Cell& c, int length, const char* s);
    String::Cell* GetNewCell();
    unsigned GarbageCollect();
    void FreeStringFamily(String::Cell& c);

    std::vector<String::Cell*> mCells;
    std::vector<CellPool*> mCellPools;
    std::vector<String::Cell*> mFreeCells;
    int mGarbageCollectCounter;
};

class StringDB {
public:
    static StringDB& GetInstance();
    void InternalizeString(const char* b, const char* e, String::StringData& out);

protected:
    StringDB();

private:
    StringDBMap mStringMap;
    unsigned mKeysCount;
    unsigned mInstancesCount;
    std::mutex mMutex;
};

}

#endif
