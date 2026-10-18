// libs/social_framework/include/social/userdata/activityfacebook.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACTIVITYFACEBOOK_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_ACTIVITYFACEBOOK_H

namespace social {
class ActivityFacebook : public social::Activity {  // sizeof 0xD4
public:
    ActivityFacebook(const social::UserSNS*);
    virtual ~ActivityFacebook();
    void OnRequestDone(bool);
protected:
    static void sOnRequestFinished(const glwebtools::UrlResponse&, void*, bool, bool);
    virtual void SaveActivity(const social::Activity::ActivityPost&);
};
}

#endif
