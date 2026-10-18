// libs/social_framework/include/social/userdata/achievementssns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACHIEVEMENTSSNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACHIEVEMENTSSNS_H

namespace social {
class AchievementsSNS : public social::Achievements {  // sizeof 0xDC
public:
    AchievementsSNS(social::UserSNS*);
    virtual ~AchievementsSNS();
protected:
    virtual void SaveAchievement(int);
private:
    static bool sOnAchievement(void*, bool, sociallib::ClientSNSEnum);
    sociallib::ClientSNSEnum m_sns;  // 0xD8
};
}

#endif
