// libs/social_framework/include/social/userdata/inboxhermes.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXHERMES_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXHERMES_H

namespace social {
class InboxHermes : public social::Inbox {  // sizeof 0x138
public:
    InboxHermes(const social::UserOsiris*);
    virtual ~InboxHermes();
protected:
    virtual void RemoveMessage(const social::MessageIn*);
    virtual void SaveMessage(social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>);
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void sOnInboxLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnHermesMessageSent(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > EncodePayload(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool DecodePayload(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_messageResponse;  // 0x104
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_currentPayload;  // 0x118
};
}

#endif
