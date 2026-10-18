// core/jet/include/video/posteffect.h
#ifndef CORE_JET_INCLUDE_VIDEO_POSTEFFECT_H
#define CORE_JET_INCLUDE_VIDEO_POSTEFFECT_H

#include <boost/noncopyable.hpp>
#include "core/pstring.h"
#include "video/material.h"

namespace jet::video {
class PostEffect : public boost::noncopyable {
public:
    explicit PostEffect(const Material&);
    explicit PostEffect(const boost::shared_ptr<RenderTechnique>&);
    explicit PostEffect(const jet::String&);
    ~PostEffect();
    boost::shared_ptr<RenderTechnique> GetRenderTechnique();
    void SetRenderTechnique(const boost::shared_ptr<RenderTechnique>&);
    void SetRenderTechnique(const jet::String&);
    Material &GetMaterial();
    const Material &GetMaterial() const;
    void SetMaterial(const Material&);
    void SetInputRenderTargetFormat(const jet::String&);
    const jet::String GetInputRenderTargetFormat();
protected:
    void SetPrev(PostEffect*);
    void CommonConstructor();
    Material m_material;
    jet::String m_preferredRTFormat;
    PostEffect *m_prev;
};
}

#endif
