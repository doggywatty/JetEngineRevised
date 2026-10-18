// libs/glwt2/include/glwebtools/tool/glwebtools_mutabledata.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_MUTABLEDATA_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_MUTABLEDATA_H

namespace glwebtools {
class MutableData : public glwebtools::NonCopyable {  // sizeof 0x14
public:
    MutableData(unsigned);
    virtual ~MutableData();
    unsigned GetSize();
    unsigned GetAllocatedSize();
    bool Resize(unsigned);
    bool Reserve(unsigned);
    bool Purge();
    bool GetData(const void*&, unsigned&);
    bool GetMutableData(void*&, unsigned&);
    bool AppendData(const void*, unsigned);
    bool Swap(glwebtools::MutableData&);
private:
    unsigned m_dataSize;  // 0x4
    unsigned m_allocatedSize;  // 0x8
    unsigned m_minAllocationSize;  // 0xC
    void* m_data;  // 0x10
};
}

#endif
