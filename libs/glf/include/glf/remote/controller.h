// libs/glf/include/glf/remote/controller.h
#ifndef LIBS_GLF_INCLUDE_GLF_REMOTE_CONTROLLER_H
#define LIBS_GLF_INCLUDE_GLF_REMOTE_CONTROLLER_H

namespace glf::remote {
class Controller : public glf::EventReceiver {  // sizeof 0x68
public:
    Controller(glf::AddrIp4);
    virtual ~Controller();
    void Draw(glf::remote::Renderer*);
    virtual bool OnEvent(const glf::CoreEvent&);
    void Update();
private:
    bool Connect();
    void DrawEvent(const glf::CoreEvent&);
    bool IsConnected();
    void ReceiveEvent(glf::ByteArrayReader<std::allocator<char> >&);
    void ReceiveIdentification(glf::ByteArrayReader<std::allocator<char> >&);
    void ReceiveMessages();
    void SendMessageA(const unsigned char*, int);
    void SendEvent(const glf::CoreEvent&);
    void SendIdentification();
    void SendQuit();
    glf::remote::Canvas *mCanvas;  // 0x8
    glf::AddrIp4 mAddrIP;  // 0xC
    bool mIsServer;  // 0x14
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mHostname;  // 0x18
    glf::Socket mSocket;  // 0x38
    __int64 mNetRetryTimeMS;  // 0x58
    bool mShowRemoteEvent;  // 0x60
    bool mShowLocalEvent;  // 0x61
    bool mIsUpdatedWithoutDraw;  // 0x62
};
}

#endif
