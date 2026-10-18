// libs/social_framework/include/social/reload/reloadstrategytime.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGYTIME_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGYTIME_H

namespace social {
class ReloadStrategyTime : public social::ReloadStrategy {  // sizeof 0xC
public:
    ReloadStrategyTime(unsigned);
    virtual void OnLoaded();
    virtual bool NeedsReload();
private:
    unsigned m_reloadTime;  // 0x4
    unsigned m_lastLoad;  // 0x8
};
}

#endif
