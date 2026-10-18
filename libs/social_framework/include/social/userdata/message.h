// libs/social_framework/include/social/userdata/message.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_MESSAGE_H

namespace logog {
class Message : public logog::Checkpoint {  // sizeof 0x118
public:
    Message(const int, const char*, const int, const char*, const char*, const char*, const double, bool*);
    virtual ~Message();
    virtual bool Republish();
    logog::Mutex m_Transmitting;  // 0xF8
    bool* m_pbIsCreated;  // 0x114
};
}

namespace social {
class Message {  // sizeof 0x1C
public:
    virtual ~Message();
    const char *GetBody();
    int GetBodySize();
    const char *GetHeader();
    int GetHeaderSize();
    bool IsInvite();
    bool IsMultiOut();
protected:
    Message(const char*, int, const char*, int, bool);
    bool m_isMultiOut;  // 0x4
private:
    char* m_body;  // 0x8
    int m_bodySize;  // 0xC
    char* m_header;  // 0x10
    int m_headerSize;  // 0x14
    bool m_isInvitation;  // 0x18
    bool m_isMultiUser;  // 0x19
};
}

#endif
