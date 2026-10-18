// libs/social_framework/include/social/userdata/inboxsns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXSNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_INBOXSNS_H

namespace social {
class InboxSNS : public social::Inbox {  // sizeof 0x104
public:
    InboxSNS(const social::UserSNS*);
    virtual ~InboxSNS();
protected:
    virtual void RemoveMessage(const social::MessageIn*);
    virtual void SaveMessage(social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock>);
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static bool sOnMessageSent(void*, bool, sociallib::ClientSNSEnum);
};
}

#endif
