// libs/social_framework/include/social/reload/reloadstrategyondemand.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGYONDEMAND_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_RELOAD_RELOADSTRATEGYONDEMAND_H

namespace social {
class ReloadStrategyOnDemand : public social::ReloadStrategy {  // sizeof 0x4
public:
    virtual void OnLoaded();
    virtual bool NeedsReload();
};
}

#endif
