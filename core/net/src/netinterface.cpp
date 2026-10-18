// core/net/src/netinterface.cpp
#include "netinterface.h"
#include <chrono>
#include <cstring>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#define NET_CLOSE closesocket
#define NET_ERRNO WSAGetLastError()
#define NET_WOULDBLOCK WSAEWOULDBLOCK
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#define NET_CLOSE ::close
#define NET_ERRNO errno
#define NET_WOULDBLOCK EWOULDBLOCK
#endif

namespace net {

static const intptr_t kInvalid = -1;

static double Seconds()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static sockaddr_in Addr(u32 ip, int port)
{
    sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(u16(port));
    a.sin_addr.s_addr = ip;
    return a;
}

static void SetNonBlocking(intptr_t fd, bool on)
{
#ifdef _WIN32
    u_long m = on ? 1 : 0;
    ioctlsocket(SOCKET(fd), FIONBIO, &m);
#else
    int f = fcntl(int(fd), F_GETFL, 0);
    fcntl(int(fd), F_SETFL, on ? (f | O_NONBLOCK) : (f & ~O_NONBLOCK));
#endif
}

bool CNetInterfaceStartUp()
{
#ifdef _WIN32
    WSADATA d;
    return WSAStartup(MAKEWORD(2, 2), &d) == 0;
#else
    return true;
#endif
}

void CNetInterfaceShutDown()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

CNetSocket::~CNetSocket()
{
    if (m_fd != kInvalid)
        NET_CLOSE(m_fd);
}

bool CNetSocket::isReadable() const
{
    fd_set s;
    FD_ZERO(&s);
    FD_SET(m_fd, &s);
    timeval tv = {0, 0};
    int r = select(int(m_fd + 1), &s, nullptr, nullptr, &tv);
    if (r < 0)
        m_error = NET_ERRNO;
    return r > 0 && FD_ISSET(m_fd, &s);
}

int CNetSocket::read(void* dst, int size, float timeout)
{
    char* p = static_cast<char*>(dst);
    int done = 0;
    double start = Seconds();
    while (size > 0) {
        int n = int(recv(m_fd, p, size, 0));
        if (n == 0)
            return done;
        if (n > 0) {
            size -= n;
            done += n;
            p += n;
            continue;
        }
        if (NET_ERRNO != NET_WOULDBLOCK) {
            m_error = NET_ERRNO;
            return done;
        }
        m_error = 0;
        if (!isReadable() && m_error)
            return done;
        if (Seconds() - start >= timeout)
            return done;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    return done;
}

int CNetSocket::write(const void* src, int size, float timeout)
{
    const char* p = static_cast<const char*>(src);
    int done = 0;
    double start = Seconds();
    while (size > 0) {
        int n = int(send(m_fd, p, size, 0));
        if (n < 0) {
            m_error = 0;
            if (Seconds() - start >= timeout)
                return done;
            continue;
        }
        size -= n;
        done += n;
        p += n;
    }
    return done;
}

void CNetSocket::setNonBlock(bool on) { SetNonBlocking(m_fd, on); }

bool CNetSocket::setNoDelay(bool on)
{
    int v = on;
    return setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&v), sizeof(v)) == 0;
}

CNetInterface::CNetInterface() : m_port(0), m_fd(kInvalid) { m_group[0] = 0; }

// Numeric hosts are used as-is when the reverse lookup fails.
CNetSocket* CNetInterface::connect(const char* host, int port)
{
    m_fd = intptr_t(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (m_fd == kInvalid)
        return nullptr;
    m_port = port;
    sockaddr_in a = Addr(0, port);
    addrinfo hints, *res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, nullptr, &hints, &res) == 0 && res) {
        a.sin_addr = reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr;
        freeaddrinfo(res);
    } else {
        a.sin_addr.s_addr = inet_addr(host);
    }
    if (::connect(m_fd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) != 0) {
        close();
        return nullptr;
    }
    setNonBlock();
    return new CNetSocket(m_fd);
}

bool CNetInterface::listen(int port)
{
    m_port = port;
    m_fd = intptr_t(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (m_fd == kInvalid)
        return false;
    sockaddr_in a = Addr(INADDR_ANY, port);
    setReuseAddress(true);
    if (bind(m_fd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) == -1 || ::listen(m_fd, 0x80) == -1) {
        close();
        return false;
    }
    setNonBlock();
    return true;
}

CNetSocket* CNetInterface::accept()
{
    intptr_t fd = intptr_t(::accept(m_fd, nullptr, nullptr));
    return fd == kInvalid || fd < 0 ? nullptr : new CNetSocket(fd);
}

void CNetInterface::close()
{
    if (m_fd != kInvalid)
        NET_CLOSE(m_fd);
    m_fd = kInvalid;
}

bool CNetInterface::isValid() const { return m_fd != kInvalid; }

bool CNetInterface::setNonBlock()
{
    SetNonBlocking(m_fd, true);
    return true;
}

bool CNetInterface::setReuseAddress(bool on)
{
    int v = on;
    return setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&v), sizeof(v)) == 0;
}

bool CNetInterface::setNoDelay(bool on)
{
    int v = on;
    return setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&v), sizeof(v)) == 0;
}

bool CNetInterface::getHostName(char* out, int size) const { return gethostname(out, size) == 0; }

bool CNetInterface::getHostAddress(char* out, int size) const
{
    if (gethostname(out, size) != 0)
        return false;
    addrinfo hints, *res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    if (getaddrinfo(out, nullptr, &hints, &res) != 0 || !res)
        return false;
    inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr, out, socklen_t(size));
    freeaddrinfo(res);
    return true;
}

bool CNetInterface::sendDatagram(const char* host, int port, const void* data, int size)
{
    if (!isValid())
        return false;
    sockaddr_in a = Addr(inet_addr(host), port);
    return sendto(m_fd, static_cast<const char*>(data), size, 0, reinterpret_cast<sockaddr*>(&a), sizeof(a)) > 0;
}

bool CNetInterface::sendMulticast(const char* group, int port, const void* data, int size)
{
    intptr_t fd = intptr_t(socket(AF_INET, SOCK_DGRAM, 0));
    if (fd == kInvalid)
        return false;
    int one = 1;
    bool ok = false;
    sockaddr_in local = Addr(INADDR_ANY, port), to = Addr(inet_addr(group), port);
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&one), sizeof(one)) >= 0 &&
        bind(fd, reinterpret_cast<sockaddr*>(&local), sizeof(local)) != -1)
        ok = sendto(fd, static_cast<const char*>(data), size, 0, reinterpret_cast<sockaddr*>(&to), sizeof(to)) == size;
    NET_CLOSE(fd);
    return ok;
}

bool CNetInterface::setupReceiveMulticast(const char* group, int port)
{
    m_fd = intptr_t(socket(AF_INET, SOCK_DGRAM, 0));
    if (m_fd != kInvalid) {
        sockaddr_in a = Addr(INADDR_ANY, port);
        ip_mreq m;
        m.imr_multiaddr.s_addr = inet_addr(group);
        m.imr_interface.s_addr = INADDR_ANY;
        if (bind(m_fd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) >= 0 &&
            setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&m), sizeof(m)) >= 0) {
            strncpy(m_group, group, sizeof(m_group) - 1);
            m_group[sizeof(m_group) - 1] = 0;
            m_port = port;
            setNonBlock();
            return true;
        }
        NET_CLOSE(m_fd);
    }
    m_fd = kInvalid;
    return false;
}

int CNetInterface::receiveMulticast(void* dst, int size)
{
    if (!isValid())
        return -1;
    sockaddr_in a = Addr(inet_addr(m_group), m_port);
    socklen_t len = sizeof(a);
    return int(recvfrom(m_fd, static_cast<char*>(dst), size, 0, reinterpret_cast<sockaddr*>(&a), &len));
}

}
