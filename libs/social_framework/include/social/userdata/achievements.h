// libs/social_framework/include/social/userdata/achievements.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACHIEVEMENTS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACHIEVEMENTS_H

namespace social {
class Achievements : public social::Storable {  // sizeof 0xD8
public:
    Achievements(social::User*);
    virtual ~Achievements();
    void NotifyAchievement(int);
protected:
    virtual void SaveImpl();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
    virtual void SaveAchievement(int) = 0;
    void OnAchievementSaved(bool);
private:
    void SaveAchievements();
    std::vector<int,std::allocator<int> > m_pendingAchievements;  // 0xBC
    int m_savingAchievement;  // 0xD0
    bool m_saveResult;  // 0xD4
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
};
}

#endif
