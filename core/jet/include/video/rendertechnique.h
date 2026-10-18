// core/jet/include/video/rendertechnique.h
#ifndef CORE_JET_INCLUDE_VIDEO_RENDERTECHNIQUE_H
#define CORE_JET_INCLUDE_VIDEO_RENDERTECHNIQUE_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "core/pstring.h"
#include "res/resource.h"
#include "video/renderpass.h"

namespace pugi { class xml_node; }

namespace jet::video {
class RenderTechnique : public jet::res::Resource {
public:
    explicit RenderTechnique(const jet::String&);
    virtual ~RenderTechnique();
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();
    virtual void Unload();
    virtual bool IsChanged();
    const jet::String &GetName() const;
    void SetName(const jet::String&);
    unsigned GetTagMask() const;
    void AddRenderPass(RenderPass*);
    unsigned GetRenderPassCount() const;
    const RenderPass &GetRenderPass(unsigned) const;
    RenderPass &GetRenderPass(unsigned);
    int FindRenderPassIdxByName(const jet::String&) const;
    bool HasDepthBufferReads() const;
    void Link();
    // SetPatches @0x6a77cc: source text put in front of every pass's vertex / fragment shader (RenderTechniqueLoader::LoadAndPatch)
    void SetPatches(const jet::String &vertex, const jet::String &fragment);
    const jet::String &GetVertexPatch() const { return m_vertexPatch; }
    const jet::String &GetFragmentPatch() const { return m_fragmentPatch; }
protected:
    // "technique" root, "rendertargets"/"target" and "renderpasses"/"pass" element names verified
    // against libdespicablemefree.so 0.0.3 (RenderTechnique::Load @0x61d8e0 reads "technique" with
    // a "version" attribute; LoadV100 @0x61d598 walks "target"/"pass" children). Render-target
    // registration itself is deferred until jet::video::Driver/RenderTarget exist.
    void LoadV100(const pugi::xml_node&);
    jet::String m_name;
    jet::String m_fragmentPatch;  // 0x10
    jet::String m_vertexPatch;  // 0x14
    std::vector<RenderPass*> m_renderPasses;
    unsigned m_tagMask;
    bool m_hasDepthBufferReads;
    bool m_readsPreviousRenderTarget;
    std::vector<boost::shared_ptr<jet::stream::IStream> > m_dependencies;
};
}

#endif
