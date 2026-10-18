// libs/social_framework/include/social/userdata/inbox.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOX_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOX_H

namespace social {
class Inbox : public social::Storable {  // sizeof 0x104
public:
    Inbox(const social::User*);
    virtual ~Inbox();
    unsigned GetNumMessages();
    const social::MessageIn *GetMsg(unsigned);
    const social::MessageIn *FindMsgByMsgId(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::MessageOutHandle SendMsg(social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>&);
    social::MessageOutHandle SendMsg(std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&, const char*, int, const char*, int, bool);
    social::MessageOutHandle SendMsg(social::User*, const char*, int, const char*, int, bool, bool);
protected:
    void RemoveMsg(const social::MessageIn*);
public:
    void RemoveMsg(unsigned);
protected:
    void RemoveMsgByMsgId(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    void RemoveMsgByMsgId(const social::MessageIn*);
    void ClearOutgoing();
protected:
    virtual void SaveMessage(social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>) = 0;
    virtual void RemoveMessage(const social::MessageIn*) = 0;
    void OnMessageSaved(bool, bool);
    virtual void SaveImpl();
    virtual void FinishSave();
    bool IsMessageInInbox(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::vector<social::MessageIn *,std::allocator<social::MessageIn *> > m_messages;  // 0xBC
    std::vector<social::MessageIn const *,std::allocator<social::MessageIn const *> > m_removingMessages;  // 0xD0
    std::vector<social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>,std::allocator<social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> > > m_outgoing;  // 0xE4
    social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> m_currentSending;  // 0xF8
    const social::MessageIn *m_currentRemoving;  // 0x100
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
};
}

#endif
