// libs/glf/include/glf/debugger/profiler.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUGGER_PROFILER_H
#define LIBS_GLF_INCLUDE_GLF_DEBUGGER_PROFILER_H

namespace glf::debugger {
class EventType {  // sizeof 0x28
public:
    EventType(const char*, bool);
    bool IsEnabled();
    void SetEnabled(bool);
private:
    std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > mContext;  // 0x0
    bool mEnabled;  // 0x20
    unsigned mID;  // 0x24
};
}

namespace glf::debugger {
class Profiler : public glf::debugger::Module, public glf::SingletonWithDep<glf::debugger::Profiler,glf::debugger::Debugger> {  // sizeof 0x8BC0
public:
    void Start();
    void Stop();
    void BeginEvent(const glf::debugger::Profiler::Event&);
    void EndEvent();
    void AddTimeEvent(const glf::debugger::Profiler::Event&, const glf::debugger::EventType&);
    void RegisterEventCallbacks(void (const char*)*, void ()*);
    void RegisterEventType(glf::debugger::EventType&);
    virtual ~Profiler();
    virtual void UpdateFrame();
    virtual void Parse(int, glf::debugger::PacketReader&);
    virtual void ConnectionOpened();
    virtual void ConnectionClosed();
private:
    Profiler();
    void ClearProfilerCollectors();
    void SendEvents();
    void SendEventTypes();
    void SetCrtDbgFrequency(int);
    glf::debugger::Profiler::ThreadInfos mThreadInfos[32];  // 0x50
    int mMainThreadID;  // 0x8B50
    bool mEnabled;  // 0x8B54
    glf::debugger::Profiler::CrtDbgFrequencyType mCrtDbgFrequencyType;  // 0x8B58
    std::vector<glf::debugger::Profiler::EventCollector *,glf::debugger::DebuggerAllocator<glf::debugger::Profiler::EventCollector *> > mProfilerEventCollectors;  // 0x8B5C
    void (*mBeginEventCallback)(const char*);  // 0x8B70
    void (*mEndEventCallback)();  // 0x8B74
    std::map<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> >,glf::debugger::EventType *,std::less<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > >,glf::debugger::DebuggerAllocator<std::pair<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > const ,glf::debugger::EventType *> > > mEventTypesMap;  // 0x8B78
    std::vector<glf::debugger::EventType *,glf::debugger::DebuggerAllocator<glf::debugger::EventType *> > mEventTypes;  // 0x8B94
    std::vector<glf::debugger::EventType *,glf::debugger::DebuggerAllocator<glf::debugger::EventType *> > mSlaveEventTypes;  // 0x8BA8
    bool mEventTypesChanged;  // 0x8BBC
};
}

namespace glf::debugger {
class StringAllocator {  // sizeof 0x18
public:
    StringAllocator();
    ~StringAllocator();
    const char *Allocate(const char*);
    void Free();
private:
    std::vector<char *,glf::debugger::DebuggerAllocator<char *> > mBlocks;  // 0x0
    int mCurrentSizeAvailable;  // 0x14
};
}

#endif
