// core/jet/src/video/rendertechnique.cpp
#include "video/rendertechnique.h"
#include "pugixml.hpp"
#include "core/util.h"
#include "video/driver.h"
#include "video/rendertarget.h"
#include "video/display.h"

namespace jet::video {

RenderTechnique::RenderTechnique(const jet::String &name)
    : m_name(name), m_tagMask(0), m_hasDepthBufferReads(false), m_readsPreviousRenderTarget(false) {
}

RenderTechnique::~RenderTechnique() { Unload(); }

bool RenderTechnique::Init(jet::stream::IStream *s) { SetStream(s); return true; }

const jet::String &RenderTechnique::GetName() const { return m_name; }
void RenderTechnique::SetName(const jet::String &n) { m_name = n; }
unsigned RenderTechnique::GetTagMask() const { return m_tagMask; }

void RenderTechnique::AddRenderPass(RenderPass *p) {
    m_renderPasses.push_back(p);
    m_tagMask |= p->GetTag();
}
unsigned RenderTechnique::GetRenderPassCount() const { return (unsigned)m_renderPasses.size(); }
const RenderPass &RenderTechnique::GetRenderPass(unsigned i) const { return *m_renderPasses[i]; }
RenderPass &RenderTechnique::GetRenderPass(unsigned i) { return *m_renderPasses[i]; }

int RenderTechnique::FindRenderPassIdxByName(const jet::String &name) const {
    for (unsigned i = 0; i < m_renderPasses.size(); i++)
        if (m_renderPasses[i]->GetName().Equals(name)) return (int)i;
    return -1;
}

void RenderTechnique::SetPatches(const jet::String &vertex, const jet::String &fragment) {
    m_vertexPatch = vertex;
    m_fragmentPatch = fragment;
}

bool RenderTechnique::HasDepthBufferReads() const { return m_hasDepthBufferReads; }

void RenderTechnique::Link() {
    for (unsigned i = 0; i < m_renderPasses.size(); i++) m_renderPasses[i]->Link();
}

bool RenderTechnique::IsChanged() {
    for (unsigned i = 0; i < m_dependencies.size(); i++)
        if (m_dependencies[i]->IsChanged()) return true;
    return jet::res::Resource::IsChanged();
}

void RenderTechnique::Unload() {
    for (unsigned i = 0; i < m_renderPasses.size(); i++) delete m_renderPasses[i];
    m_renderPasses.clear();
    m_dependencies.clear();
}

// LoadV100 @0x6a6f74: render targets the technique needs, then the passes whose target is the
// driver's architecture ("GLES2.0") or unspecified; a pass sampling "#previous" makes the
// technique read the previous render target. The technique links once loaded.
void RenderTechnique::LoadV100(const pugi::xml_node &node) {
    Driver *driver = Driver::GetCurrent();
    for (pugi::xml_node rts = node.child("rendertargets"); rts; rts = rts.next_sibling("rendertargets"))
        for (pugi::xml_node t = rts.child("target"); t; t = t.next_sibling("target")) {
            jet::String name(t.attribute("name").value());
            if (!driver || driver->FindRenderTargetByName(name)) continue;
            boost::shared_ptr<RenderTarget> rt = RenderTarget::New();
            math::vec2<unsigned int> size(t.attribute("width").as_uint(), t.attribute("height").as_uint());
            if (pugi::xml_attribute r = t.attribute("screenRatio")) {
                math::vec2<unsigned int> ds = driver->GetDisplay() ? driver->GetDisplay()->GetSize() : math::vec2<unsigned int>(0, 0);
                size = math::vec2<unsigned int>((unsigned)(ds.x * r.as_float()), (unsigned)(ds.y * r.as_float()));
            }
            rt->Create(name, jet::String(t.attribute("format").value()), size);
            driver->AddRenderTarget(rt);
        }
    jet::String arch = driver ? driver->GetArchitectureString() : jet::String("GLES2.0");
    for (pugi::xml_node p = node.child("pass"); p; p = p.next_sibling("pass")) {
        pugi::xml_attribute target = p.attribute("target");
        if (target && !jet::String(target.value()).Equals(arch)) continue;
        RenderPass *pass = new RenderPass(this, jet::String());
        pass->Load(p);
        AddRenderPass(pass);
        const ShaderData &auto_ = pass->GetAutoShaderData();
        for (unsigned i = 0; i < auto_.GetSamplerCount(); i++)
            if (auto_.GetSampler(i).GetBinding().Equals("#previous")) m_readsPreviousRenderTarget = true;
        if (pass->GetShaderProgram()) pass->GetShaderProgram()->SetDebugName(m_name);
    }
    Link();
}

bool RenderTechnique::Load() {
    jet::stream::IStream *s = GetStream();
    s->Start();
    unsigned size = s->Size();
    std::string buf;
    buf.resize(size);
    if (size) s->Read(&buf[0], size);

    // #include resolution reuses the same preprocessor as shaders (jet::core::ResolveIncludes);
    // dependency tracking (m_dependencies) is skipped until StreamMgr's by-name stream lookup is
    // wired up the same way the original does in this loop.
    std::set<std::string> included;
    std::string resolved;
    jet::core::ResolveIncludes(buf, resolved, included);

    pugi::xml_document doc;
    bool ok = doc.load_buffer(resolved.c_str(), resolved.size()).status == pugi::status_ok;
    if (ok) {
        pugi::xml_node root = doc.child("technique");
        pugi::xml_attribute versionAttr = root.attribute("version");
        int version = versionAttr ? versionAttr.as_int() : 100;
        if (version == 100) LoadV100(root);
    }
    SetLoaded(true);
    s->Finish();
    return ok;
}

}
