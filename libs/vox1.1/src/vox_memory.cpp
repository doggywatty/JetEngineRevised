// libs/vox1.1/src/vox_memory.cpp
#include "vox_memory.h"

// @0x63b890
void *VoxAlloc(unsigned size, vox::VoxMemHint, const char *, const char *, int) { return malloc(size ? size : 1); }
// @0x63b888
void *VoxAlloc(unsigned size, const char *, const char *, int) { return malloc(size ? size : 1); }
// @0x63b88c
void *VoxAlloc(unsigned size) { return malloc(size ? size : 1); }
// @0x63b8a0
void *VoxAlloc(unsigned size, vox::VoxMemHint) { return malloc(size ? size : 1); }
// @0x63b8a4
void VoxFree(void *p) { free(p); }
