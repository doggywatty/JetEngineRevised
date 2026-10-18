// libs/social_framework/include/social/userdata/activity.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACTIVITY_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACTIVITY_H

namespace social {
class Activity : public social::Storable {  // sizeof 0xD4
public:
    Activity(const social::User*);
    virtual ~Activity();
    void Publish(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void PublishAchievement(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void PublishScore(int);
protected:
    virtual void SaveActivity(const social::Activity::ActivityPost&) = 0;
    void OnActivitySaved(bool);
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
    virtual void SaveImpl();
private:
    std::vector<social::Activity::ActivityPost,std::allocator<social::Activity::ActivityPost> > m_activities;  // 0xBC
    bool m_success;  // 0xD0
};
}

#endif
