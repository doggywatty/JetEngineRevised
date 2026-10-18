// core/jet/src/video/posteffect.cpp
#include "video/posteffect.h"
#include <boost/pointer_cast.hpp>

namespace jet::video {

void PostEffect::CommonConstructor() { m_prev = 0; }

PostEffect::PostEffect(const Material &m) : m_material(m) { CommonConstructor(); }
PostEffect::PostEffect(const boost::shared_ptr<RenderTechnique> &tech) { CommonConstructor(); SetRenderTechnique(tech); }
PostEffect::PostEffect(const jet::String &techName) { CommonConstructor(); SetRenderTechnique(techName); }
PostEffect::~PostEffect() {}

boost::shared_ptr<RenderTechnique> PostEffect::GetRenderTechnique() {
    return boost::const_pointer_cast<RenderTechnique>(m_material.GetRenderTechnique());
}
void PostEffect::SetRenderTechnique(const boost::shared_ptr<RenderTechnique> &tech) { m_material.SetRenderTechnique(tech); }
void PostEffect::SetRenderTechnique(const jet::String &name) { m_material.SetRenderTechnique(name); }

Material &PostEffect::GetMaterial() { return m_material; }
const Material &PostEffect::GetMaterial() const { return m_material; }
void PostEffect::SetMaterial(const Material &m) { m_material = m; }

void PostEffect::SetInputRenderTargetFormat(const jet::String &f) { m_preferredRTFormat = f; }
const jet::String PostEffect::GetInputRenderTargetFormat() { return m_preferredRTFormat; }

void PostEffect::SetPrev(PostEffect *p) { m_prev = p; }

}
