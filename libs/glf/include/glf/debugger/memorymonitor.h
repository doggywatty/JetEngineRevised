// libs/glf/include/glf/debugger/memorymonitor.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUGGER_MEMORYMONITOR_H
#define LIBS_GLF_INCLUDE_GLF_DEBUGGER_MEMORYMONITOR_H

namespace glf::debugger {
class MemoryMonitor : protected glf::debugger::MemoryContainer, public glf::debugger::Module, public glf::SingletonWithDep<glf::debugger::MemoryMonitor,glf::debugger::Debugger> {  // sizeof 0xE0
public:
    static void RegisterAllocation(void*, unsigned, const char*, unsigned, unsigned);
    static void UnregisterAllocation(void*, unsigned);
    static void SetRecordCallStack(bool);
    static void PushMemoryContext(const char*);
    static void PopMemoryContext();
    static unsigned RegisterAllocator(const char*);
    static void SetAllocatorCapacity(unsigned, unsigned);
    static void CreateSnapshot(const char*);
    static void CreateSnapshotLeaks(const char*);
    static unsigned GetTotalAllocated(unsigned);
    static unsigned __int64 GetBaseTime();
    virtual void UpdateFrame();
    virtual void Parse(int, glf::debugger::PacketReader&);
    virtual void DebuggerDestroyed();
private:
    MemoryMonitor();
    virtual ~MemoryMonitor();
    void RegisterAllocationInternal(void*, unsigned, const char*, unsigned, unsigned);
    void UnregisterAllocationInternal(void*, unsigned);
    unsigned GetTotalAllocatedInternal(unsigned);
    glf::debugger::MemoryContainer::SContext *CreateContext(const char*);
    void PushContext(const char*);
    void PopContext();
    unsigned CreateAllocator(const char*);
    glf::debugger::MemoryContainer::SThreadContextStack &GetThreadContextStack();
    void SendSnapshot(const char*);
    void SendMemoryMonitorRefresh();
    void SendMemoryMonitorSnapshot(glf::debugger::MemoryContainer*, const char*, bool, bool);
    void SendMemoryLeaks(const char*);
    void SendMemoryMonitorActivity();
    void ResetPerformanceCounters();
    static void InitBaseTime();
    static void InitBaseTime(glf::debugger::MemoryMonitor*);
    static void ReleaseInstance();
    static void Close();
    static void* operator new(unsigned);
    static void operator delete(void*);
public:
    static const unsigned SYSTEM_ALLOCATOR;
private:
    bool RecordCallStack;  // 0xA8
    unsigned __int64 BaseTime;  // 0xB0
    std::vector<glf::debugger::MemoryMonitor::SEvent,glf::debugger::DebuggerAllocator<glf::debugger::MemoryMonitor::SEvent> > ActivityEvents;  // 0xB8
    bool RecordActivity;  // 0xCC
    int FrameAllocationCount;  // 0xD0
    int FrameDeallocationCount;  // 0xD4
    int FrameAllocatedHeap;  // 0xD8
    int FrameDeallocatedHeap;  // 0xDC
};
}

#endif
