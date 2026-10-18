// core/net/include/netinterface.h
#ifndef CORE_NET_INCLUDE_NETINTERFACE_H
#define CORE_NET_INCLUDE_NETINTERFACE_H

#include "jtypes.h"

namespace net {

bool CNetInterfaceStartUp();  // Winsock init on Windows; no-op elsewhere
void CNetInterfaceShutDown();

// A connected TCP socket (owns the descriptor).
class CNetSocket {
public:
    explicit CNetSocket(intptr_t fd) : m_fd(fd), m_error(0) {}
    ~CNetSocket();
    CNetSocket(const CNetSocket&) = delete;
    CNetSocket& operator=(const CNetSocket&) = delete;

    int getError() const { return m_error; }
    bool isOpen() const { return m_error == 0; }
    bool isReadable() const;
    // Loop until size bytes moved, the peer closes, an error, or timeout seconds pass.
    int read(void* dst, int size, float timeout);
    int write(const void* src, int size, float timeout);
    void setNonBlock(bool on);
    bool setNoDelay(bool on);

private:
    intptr_t m_fd;
    mutable int m_error;
};

// Client/server/multicast helper around one socket.
class CNetInterface {
public:
    CNetInterface();
    ~CNetInterface() { close(); }
    CNetInterface(const CNetInterface&) = delete;
    CNetInterface& operator=(const CNetInterface&) = delete;

    CNetSocket* connect(const char* host, int port);  // caller owns the result
    bool listen(int port);
    CNetSocket* accept();
    void close();
    bool isValid() const;
    bool setNonBlock();
    bool setReuseAddress(bool on);
    bool setNoDelay(bool on);
    bool getHostName(char* out, int size) const;
    bool getHostAddress(char* out, int size) const;
    bool sendDatagram(const char* host, int port, const void* data, int size);
    static bool sendMulticast(const char* group, int port, const void* data, int size);
    bool setupReceiveMulticast(const char* group, int port);
    int receiveMulticast(void* dst, int size);

private:
    char m_group[64];
    int m_port;
    intptr_t m_fd;
};

}

#endif
