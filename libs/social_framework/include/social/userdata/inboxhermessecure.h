// libs/social_framework/include/social/userdata/inboxhermessecure.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXHERMESSECURE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXHERMESSECURE_H

namespace social {
class InboxHermesSecure : public social::Inbox {  // sizeof 0x118
public:
    InboxHermesSecure(const social::UserOsiris*);
    virtual ~InboxHermesSecure();
protected:
    virtual void RemoveMessage(const social::MessageIn*);
    virtual void SaveMessage(social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>);
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
