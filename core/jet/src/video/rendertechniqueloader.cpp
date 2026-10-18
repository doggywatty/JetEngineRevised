// core/jet/src/video/rendertechniqueloader.cpp
#include "video/rendertechniqueloader.h"
#include "stream/streammgr.h"
#include <cstring>

namespace jet::video {

RenderTechniqueLoader *RenderTechniqueLoader::s_instance = 0;

RenderTechniqueLoader::RenderTechniqueLoader() {}
RenderTechniqueLoader::~RenderTechniqueLoader() {}

RenderTechniqueLoader *RenderTechniqueLoader::GetInstance() {
    if (!s_instance) s_instance = new RenderTechniqueLoader();
    return s_instance;
}
void RenderTechniqueLoader::FreeInstance() { delete s_instance; s_instance = 0; }

unsigned RenderTechniqueLoader::GetLoadedTechniquesCount() const { return (unsigned)m_map.size(); }

const boost::shared_ptr<RenderTechnique> RenderTechniqueLoader::GetLoadedTechnique(unsigned i) const {
    boost::unordered_map<jet::String, boost::shared_ptr<RenderTechnique> >::const_iterator it = m_map.begin();
    for (unsigned n = 0; n < i && it != m_map.end(); n++) ++it;
    return it == m_map.end() ? boost::shared_ptr<RenderTechnique>() : it->second;
}

bool RenderTechniqueLoader::CanLoad(const jet::String &name) const {
    return jet::stream::StreamMgr::GetInstance().CanCreateStream(name);
}

// @0x61e55c: a name without an extension is the technique's .xml
boost::shared_ptr<RenderTechnique> RenderTechniqueLoader::Load(const jet::String &requested) {
    jet::String name = requested;
    if (!strchr(name.c_str(), '.')) name.append(".xml");
    boost::unordered_map<jet::String, boost::shared_ptr<RenderTechnique> >::iterator it = m_map.find(name);
    if (it != m_map.end()) return it->second;

    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) return boost::shared_ptr<RenderTechnique>();

    boost::shared_ptr<RenderTechnique> tech(new RenderTechnique(name));
    tech->Init(s);
    if (!tech->Load()) return boost::shared_ptr<RenderTechnique>();  // Load links the passes
    m_map[name] = tech;
    return tech;
}

// @0x6a7e44
boost::shared_ptr<RenderTechnique> RenderTechniqueLoader::LoadAndPatch(const jet::String &requested, const jet::String &vertexPatch, const jet::String &fragmentPatch) {
    jet::String name = requested;
    if (!strchr(name.c_str(), '.')) name.append(".xml");
    const std::string key = std::string(name.c_str()) + '\n' + vertexPatch.c_str() + '\n' + fragmentPatch.c_str();
    std::map<std::string, boost::shared_ptr<RenderTechnique> >::iterator it = m_patched.find(key);
    if (it != m_patched.end()) return it->second;
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) return boost::shared_ptr<RenderTechnique>();
    boost::shared_ptr<RenderTechnique> tech(new RenderTechnique(name));
    tech->SetPatches(vertexPatch, fragmentPatch);
    tech->Init(s);
    if (!tech->Load()) return boost::shared_ptr<RenderTechnique>();
    m_patched[key] = tech;
    return tech;
}

void RenderTechniqueLoader::Free(boost::shared_ptr<RenderTechnique> &tech) {
    if (!tech) return;
    m_map.erase(tech->GetName());
    tech.reset();
}

void RenderTechniqueLoader::FreeAll() { m_map.clear(); m_patched.clear(); }

bool RenderTechniqueLoader::LoadAll(unsigned &idx, unsigned count) {
    std::vector<jet::String> files = jet::stream::StreamMgr::GetInstance().GetFileList(".rendertechnique");
    unsigned end = std::min((unsigned)files.size(), idx + count);
    for (; idx < end; idx++) Load(files[idx]);
    return idx >= files.size();
}
bool RenderTechniqueLoader::LoadAll() {
    unsigned idx = 0;
    return LoadAll(idx, 0xffffffff);
}

void RenderTechniqueLoader::UnloadAll() {
    for (boost::unordered_map<jet::String, boost::shared_ptr<RenderTechnique> >::iterator it = m_map.begin(); it != m_map.end(); ++it)
        it->second->Unload();
    for (std::map<std::string, boost::shared_ptr<RenderTechnique> >::iterator it = m_patched.begin(); it != m_patched.end(); ++it)
        it->second->Unload();
}

void RenderTechniqueLoader::ReloadAllChanged() {
    for (boost::unordered_map<jet::String, boost::shared_ptr<RenderTechnique> >::iterator it = m_map.begin(); it != m_map.end(); ++it) {
        if (it->second->IsChanged()) { it->second->Unload(); it->second->Load(); it->second->Link(); }
    }
    for (std::map<std::string, boost::shared_ptr<RenderTechnique> >::iterator it = m_patched.begin(); it != m_patched.end(); ++it)
        if (it->second->IsChanged()) { it->second->Unload(); it->second->Load(); it->second->Link(); }
}

}
