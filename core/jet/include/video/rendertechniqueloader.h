// core/jet/include/video/rendertechniqueloader.h
#ifndef CORE_JET_INCLUDE_VIDEO_RENDERTECHNIQUELOADER_H
#define CORE_JET_INCLUDE_VIDEO_RENDERTECHNIQUELOADER_H

#include <vector>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include "core/pstring.h"
#include "video/rendertechnique.h"

namespace jet::video {
class RenderTechniqueLoader : public boost::noncopyable {
protected:
    RenderTechniqueLoader();
    ~RenderTechniqueLoader();
public:
    static RenderTechniqueLoader *GetInstance();
    static void FreeInstance();
    unsigned GetLoadedTechniquesCount() const;
    const boost::shared_ptr<RenderTechnique> GetLoadedTechnique(unsigned) const;
    bool CanLoad(const jet::String&) const;
    boost::shared_ptr<RenderTechnique> Load(const jet::String&);
    // @0x6a7e44: the technique with the given texts in front of its vertex / fragment shaders, one per (name, patches)
    boost::shared_ptr<RenderTechnique> LoadAndPatch(const jet::String &name, const jet::String &vertexPatch, const jet::String &fragmentPatch);
    void Free(boost::shared_ptr<RenderTechnique>&);
    void FreeAll();
    bool LoadAll(unsigned&, unsigned);
    bool LoadAll();
    void UnloadAll();
    void ReloadAllChanged();
private:
    static RenderTechniqueLoader *s_instance;
    boost::unordered_map<jet::String, boost::shared_ptr<RenderTechnique> > m_map;
    std::map<std::string, boost::shared_ptr<RenderTechnique> > m_patched;  // key: name, vertex patch, fragment patch (ShaderKey)
};
}

#endif
