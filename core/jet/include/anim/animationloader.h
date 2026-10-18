// core/jet/include/anim/animationloader.h
#ifndef CORE_JET_INCLUDE_ANIM_ANIMATIONLOADER_H
#define CORE_JET_INCLUDE_ANIM_ANIMATIONLOADER_H

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "core/pstring.h"

namespace jet::anim {
class Animation;

// AnimationLoader (Load @0x7230e0): animations cached by file name
class AnimationLoader {
protected:
    AnimationLoader();
    ~AnimationLoader();
public:
    static AnimationLoader *GetInstance();
    static void FreeInstance();
    Animation *Load(const jet::String&);
    void Free(Animation*) {}
    void FreeAll();
    void UnloadAll();
private:
    static AnimationLoader *s_instance;
    boost::unordered_map<jet::String, boost::shared_ptr<Animation> > m_map;
};
}

#endif
