// libs/social_framework/include/social/reload/reloadstrategynull.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGYNULL_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGYNULL_H

namespace social {
class ReloadStrategyNULL : public social::ReloadStrategy {  // sizeof 0x4
public:
    virtual void OnLoaded();
    virtual bool NeedsReload();
};
}

#endif
