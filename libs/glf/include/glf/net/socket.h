// libs/glf/include/glf/net/socket.h
#ifndef LIBS_GLF_INCLUDE_GLF_NET_SOCKET_H
#define LIBS_GLF_INCLUDE_GLF_NET_SOCKET_H

namespace glf {
class Socket {  // sizeof 0x20
public:
    static glf::NetError Init();
    static glf::NetError Deinit();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetHostName();
    static const char *GetAddrIP(const glf::AddrIp4&);
    static glf::AddrIp4 MakeAddr(const char*, int);
    Socket();
    virtual ~Socket();
    bool IsOpened();
    bool OpenTcp(glf::Socket::Option);
    bool OpenUdp(glf::Socket::Option);
    bool Open(glf::Socket::Type, glf::Socket::Proto, glf::Socket::Option);
    void Close();
    bool Connect(const glf::AddrIp4&);
    bool IsConnected();
    bool Bind(const glf::AddrIp4&);
    bool Listen(int);
    int Accept(glf::AddrIp4&);
    bool Reject(int);
    int Send(const void*, int, int);
    int SendTo(const glf::AddrIp4&, const void*, int);
    bool IsReadable(int);
    int Receive(void*, int, int);
    int ReceiveFrom(glf::AddrIp4&, void*, int);
    bool GetAddress(glf::AddrIp4&);
    glf::NetError GetLastError();
    void ClearError();
    glf::Socket::Impl &GetImpl();
private:
    glf::Socket::Type mType;  // 0x4
    glf::Socket::Proto mProtocol;  // 0x8
    glf::Socket::Option mOptions;  // 0xC
    int mConnectedCount;  // 0x10
    bool mIsOpened;  // 0x14
    glf::NetError mLastError;  // 0x18
    glf::Socket::Impl &m;  // 0x1C
};
}

namespace glwebtools {
class Socket : public glwebtools::NonCopyable {  // sizeof 0x8
public:
    static glwebtools::Socket::Error Init();
    static glwebtools::Socket::Error Deinit();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetHostName();
    static const char *GetAddrIP(const glwebtools::AddrIpv4&);
    static glwebtools::AddrIpv4 MakeAddr(const char*, int);
    static int ResolveHostTCP(const char*, int, glwebtools::AddrIpv4&);
    Socket();
    virtual ~Socket();
    bool IsOpened();
    bool OpenTcp(glwebtools::Socket::Option);
    bool OpenUdp(glwebtools::Socket::Option);
    bool Open(glwebtools::Socket::Type, glwebtools::Socket::Proto, glwebtools::Socket::Option);
    void Close();
    bool Connect(const glwebtools::AddrIpv4&);
    bool IsConnected();
    bool Bind(const glwebtools::AddrIpv4&);
    bool Listen(int);
    int Accept(glwebtools::AddrIpv4&);
    bool Reject(int);
    int Send(const void*, int, int);
    int SendTo(const glwebtools::AddrIpv4&, const void*, int);
    bool IsReadable(int);
    int Receive(void*, int, int);
    int ReceiveFrom(glwebtools::AddrIpv4&, void*, int);
    bool GetAddress(glwebtools::AddrIpv4&);
    glwebtools::Socket::Error GetLastError();
    void ClearError();
    static bool GetMacAddressFromIndex(unsigned, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool GetMacAddressFromName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool GetNetInterfaceInfoArray(std::vector<glwebtools::NetInterfaceInfo,glwebtools::SAllocator<glwebtools::NetInterfaceInfo,4> >&);
private:
    void* m_handler;  // 0x4
};
}

#endif
