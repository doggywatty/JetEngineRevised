// libs/glf/include/glf/debugger/network.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUGGER_NETWORK_H
#define LIBS_GLF_INCLUDE_GLF_DEBUGGER_NETWORK_H

namespace glf::debugger {
class Network : public glf::debugger::Module, public glf::SingletonWithDep<glf::debugger::Network,glf::debugger::Debugger> {  // sizeof 0x68
public:
    static void AddEvent(const glf::debugger::Network::Event&);
    virtual ~Network();
    virtual void UpdateFrame();
    virtual void Parse(int, glf::debugger::PacketReader&);
    virtual void ConnectionClosed();
private:
    Network();
    void SendEvents();
    bool mEnabled;  // 0x50
    std::vector<glf::debugger::Network::PendingEvent,glf::debugger::DebuggerAllocator<glf::debugger::Network::PendingEvent> > mPendingEvents;  // 0x54
};
}

#endif
