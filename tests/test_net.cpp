#undef NDEBUG
#include "netinterface.h"
#include "packet.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <thread>

using namespace net;

int main()
{
    CNetInterfaceStartUp();
    CNetInterface server;
    assert(server.listen(47123));
    CNetInterface client;
    CNetSocket* c = client.connect("127.0.0.1", 47123);
    assert(c);
    CNetSocket* s = nullptr;
    for (int i = 0; i < 200 && !s; ++i) {
        s = server.accept();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    assert(s);
    PacketWriter w(0x1234);
    w.Write(u8(7));
    w.Write(0xdeadbeefu);
    w.Write("minion");
    w.Send(c);
    PacketReader r;
    int n = r.Receive(s, 2.0f);
    assert(n == int(r.GetSize()) && r.GetType() == 0x1234);
    assert(r.ReadByte() == 7 && r.ReadIntLE() == 0xdeadbeef && r.ReadIntLE() == 7);
    char buf[7];
    r.Read(reinterpret_cast<u8*>(buf), 7);
    assert(std::string(buf) == "minion");
    delete c;
    delete s;
    CNetInterfaceShutDown();
    puts("net ok");
}
