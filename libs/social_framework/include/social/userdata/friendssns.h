// libs/social_framework/include/social/userdata/friendssns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDSSNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDSSNS_H

namespace social {
class FriendsSNS : public social::Friends {  // sizeof 0x120
public:
    FriendsSNS(sociallib::ClientSNSEnum, const social::User*);
    virtual ~FriendsSNS();
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
private:
    static bool sOnPlayingFriendsLoaded(void*, bool, sociallib::ClientSNSEnum);
    static bool sOnNonPlayingFriendsLoaded(void*, bool, sociallib::ClientSNSEnum);
    sociallib::ClientSNSEnum m_sns;  // 0x11C
};
}

#endif
