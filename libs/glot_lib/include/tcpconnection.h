// libs/glot_lib/include/tcpconnection.h
#ifndef LIBS_GLOT_LIB_INCLUDE_TCPCONNECTION_H
#define LIBS_GLOT_LIB_INCLUDE_TCPCONNECTION_H

namespace glot {
class TCPConnection {  // sizeof 0x434
public:
    TCPConnection();
    virtual ~TCPConnection();
    void ServerInitCommuncation(const int&);
    int ServerInitSocket(const int&);
    int UpdateCommunication();
    int ServerAcceptConnections();
    void CloseCommunication();
    int SendData(const char*, unsigned);
    void ReceiveData();
    void SetConnectionState(int);
    int GetConnectionState();
    bool IsConnected();
    bool IsConnectionInitialized();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_host;  // 0x4
    int m_port;  // 0x24
    int m_connectionState;  // 0x28
    int m_socketFd;  // 0x2C
    int m_clientSockFd;  // 0x30
    char m_recvDataBuff[1024];  // 0x34
};
}

#endif
