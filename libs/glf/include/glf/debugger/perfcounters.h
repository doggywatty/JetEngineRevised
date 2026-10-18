// libs/glf/include/glf/debugger/perfcounters.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUGGER_PERFCOUNTERS_H
#define LIBS_GLF_INCLUDE_GLF_DEBUGGER_PERFCOUNTERS_H

namespace glf::debugger {
class PerfCounters : public glf::debugger::Module, public glf::SingletonWithDep<glf::debugger::PerfCounters,glf::debugger::Debugger> {  // sizeof 0x74
public:
    static void UpdateValue(const char*, unsigned);
    virtual ~PerfCounters();
    virtual void UpdateFrame();
    virtual void Parse(int, glf::debugger::PacketReader&);
    static const std::map<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> >,glf::debugger::PerfCounters::Counter,std::less<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > >,glf::debugger::DebuggerAllocator<std::pair<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > const ,glf::debugger::PerfCounters::Counter> > > &GetCounters();
private:
    PerfCounters();
    void SendPerformanceCounters();
    std::map<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> >,glf::debugger::PerfCounters::Counter,std::less<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > >,glf::debugger::DebuggerAllocator<std::pair<std::basic_string<char,std::char_traits<char>,glf::debugger::DebuggerAllocator<char> > const ,glf::debugger::PerfCounters::Counter> > > mCounters;  // 0x50
    bool mEnabled;  // 0x6C
    unsigned mPosition;  // 0x70
};
}

#endif
