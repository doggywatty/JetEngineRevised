// libs/social_framework/include/social/userdata/inboxhermesanonymous.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXHERMESANONYMOUS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXHERMESANONYMOUS_H

namespace social {
class InboxHermesAnonymous : public social::Inbox {  // sizeof 0x118
public:
    InboxHermesAnonymous(const social::UserOsiris*);
    virtual ~InboxHermesAnonymous();
protected:
    virtual void RemoveMessage(const social::MessageIn*);
    void SaveMessage(const social::MessageOut*);
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void sOnInboxLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnHermesMessageRemoved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_messageResponse;  // 0x104
};
}

#endif
