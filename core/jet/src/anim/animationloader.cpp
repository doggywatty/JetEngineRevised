// core/jet/src/anim/animationloader.cpp
#include "anim/animationloader.h"
#include "anim/animation.h"
#include "stream/streammgr.h"
#include <cstdio>

namespace jet::anim {

AnimationLoader *AnimationLoader::s_instance = 0;

AnimationLoader::AnimationLoader() {}
AnimationLoader::~AnimationLoader() { FreeAll(); }
AnimationLoader *AnimationLoader::GetInstance() {
    if (!s_instance) s_instance = new AnimationLoader();
    return s_instance;
}
void AnimationLoader::FreeInstance() { delete s_instance; s_instance = 0; }
void AnimationLoader::FreeAll() { m_map.clear(); }
void AnimationLoader::UnloadAll() {
    for (boost::unordered_map<jet::String, boost::shared_ptr<Animation> >::iterator it = m_map.begin(); it != m_map.end(); ++it)
        if (it->second) it->second->Unload();
}

// Load @0x7230e0: the header is read now, the keys on first sample
Animation *AnimationLoader::Load(const jet::String &name) {
    if (name.empty()) return 0;
    boost::unordered_map<jet::String, boost::shared_ptr<Animation> >::iterator it = m_map.find(name);
    if (it != m_map.end()) return it->second.get();
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) {
        fprintf(stderr, "ERROR: could not find animation %s\n", name.c_str());
        m_map[name] = boost::shared_ptr<Animation>();
        return 0;
    }
    boost::shared_ptr<Animation> a(new Animation());
    a->SetResourceName(name);
    if (!a->Init(s)) {
        m_map[name] = boost::shared_ptr<Animation>();
        return 0;
    }
    m_map[name] = a;
    return a.get();
}

}
