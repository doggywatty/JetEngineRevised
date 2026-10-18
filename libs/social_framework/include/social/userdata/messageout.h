// libs/social_framework/include/social/userdata/messageout.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGEOUT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGEOUT_H

namespace social {
class MessageOut : public social::Message, public social::RefCounted {  // sizeof 0x54
public:
    virtual ~MessageOut();
    static social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> Create(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, int, const char*, int, bool, bool);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetTo();
    bool MustSendAlert();
    social::MessageOut::Status GetStatus();
    void SetStatus(social::MessageOut::Status);
    const social::message::Result &GetResult();
    social::message::Result &GetResult();
protected:
    MessageOut(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, int, const char*, int, bool, bool);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_toUid;  // 0x24
    bool m_mustSendAlert;  // 0x44
    social::MessageOut::Status m_status;  // 0x48
    social::message::Result m_result;  // 0x4C
};
}

namespace social {
class MessageOutHandle {  // sizeof 0xC
    MessageOutHandle(const social::Inbox&, social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>);
public:
    MessageOutHandle(const social::Inbox&);
    ~MessageOutHandle();
    bool IsValid();
    social::MessageOut::Status GetStatus();
    const social::message::Result &GetResult();
    social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> m_msg;  // 0x0
    const social::Inbox &m_inbox;  // 0x8
    static social::message::Result k_default_result;
};
}

namespace social::message {
class Result {  // sizeof 0x8
public:
    Result();
    ~Result();
    void SetReponseCode(int);
    int GetResponseCode();
    void SetError(social::message::Error);
    social::message::Error GetError();
    bool IsError();
private:
    social::message::Error m_error;  // 0x0
    int m_responseCode;  // 0x4
};
}

#endif
