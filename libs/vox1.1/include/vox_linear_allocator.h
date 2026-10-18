// libs/vox1.1/include/vox_linear_allocator.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_LINEAR_ALLOCATOR_H
#define LIBS_VOX1_1_INCLUDE_VOX_LINEAR_ALLOCATOR_H

namespace vox {
class LinAlloc {  // sizeof 0x10
public:
    LinAlloc();
    ~LinAlloc();
    void AddSize(int, int);
    bool Allocate();
    char* GetMem(int, int);
private:
    char* data;  // 0x0
    char* next;  // 0x4
    int size;  // 0x8
    int maxAlign;  // 0xC
};
}

#endif
