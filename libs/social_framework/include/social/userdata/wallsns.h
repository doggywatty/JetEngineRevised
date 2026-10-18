// libs/social_framework/include/social/userdata/wallsns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_WALLSNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_WALLSNS_H

namespace social {
class WallSNS : public social::Wall {  // sizeof 0xE4
public:
    WallSNS(const social::UserSNS*, sociallib::ClientSNSEnum);
    virtual ~WallSNS();
protected:
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
    virtual void SavePost(const social::Wall::SWallPost&);
private:
    static bool sOnPostFinished(void*, bool, sociallib::ClientSNSEnum);
    const social::UserSNS *m_owner;  // 0xDC
    sociallib::ClientSNSEnum m_sns;  // 0xE0
};
}

#endif
