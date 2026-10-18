// libs/glwt2/include/glwebtools/internal/glwebtools_handlemanager.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_INTERNAL_GLWEBTOOLS_HANDLEMANAGER_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_INTERNAL_GLWEBTOOLS_HANDLEMANAGER_H

namespace glwebtools {
class HandleManager : public glwebtools::NonCopyable {  // sizeof 0x24
public:
    static glwebtools::HandleManager *GetInstance();
    bool RegisterNode(unsigned, unsigned, void*, unsigned&);
    bool UnregisterNode(unsigned);
    bool GetObjectPointer(unsigned, void*&);
    static bool RegisterType(unsigned&);
    static unsigned GetNextCounterValue();
private:
    HandleManager();
    std::vector<glwebtools::HandleManagerNode,glwebtools::SAllocator<glwebtools::HandleManagerNode,4> > m_handleNodes;  // 0x4
    glwebtools::Mutex m_mutex;  // 0x18
    unsigned m_freeIndex;  // 0x20
    static unsigned s_nextTypeId;
    static unsigned s_nextCounter;
    static glwebtools::HandleManager *s_instance;
};
}

namespace glwebtools {
struct HandleManagerNode {  // sizeof 0xC
    HandleManagerNode();
    unsigned m_token;  // 0x0
    bool m_isEmpty;  // 0x4
    unsigned m_objectPointer;  // 0x8
};
}

#endif
