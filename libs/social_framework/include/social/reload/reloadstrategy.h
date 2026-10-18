// libs/social_framework/include/social/reload/reloadstrategy.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGY_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGY_H

namespace social {
class ReloadStrategy {  // sizeof 0x4
public:
    virtual ~ReloadStrategy();
    virtual void OnLoaded() = 0;
    virtual bool NeedsReload() = 0;
};
}

#endif
