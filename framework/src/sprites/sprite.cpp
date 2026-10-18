#include <cstdio>
#include <cstdlib>
// framework/src/sprites/sprite.cpp
#include "sprites/sprite.h"
#include "stream/streammgr.h"
#include "video/textureloader.h"
#include "video/texturelayer.h"
#include "video/renderstate.h"
#include "video/blendformula.h"

Sprite::SpriteBlending Sprite::s_crtBlending = Sprite::SpriteBlending(Sprite::SpriteBlendingDef::Alpha);
float Sprite::s_posScale = 1.0f;
float Sprite::s_textureScale = 1.0f;

Sprite::Sprite() : m_posScale(s_posScale), m_textureScale(s_textureScale), m_canLoadWithoutTGA(false), m_tempMaterial(0) {}
Sprite::~Sprite() {}

void Sprite::Unload() { ASprite::Unload(); }

// The shared material every sprite paints with: technique "sprite2d.xml", sampler "texture0"
void Sprite::SetOverridenRenderTechnique(const jet::String &name) {
    GetMaterial().SetRenderTechnique(name.empty() ? jet::String("sprite2d.xml") : name);
}

jet::video::Material &Sprite::GetMaterial() {
    static jet::video::Material s_spriteMaterial;
    static bool s_init = false;
    if (!s_init) {
        s_spriteMaterial.SetRenderTechnique(jet::String("sprite2d.xml"));
        s_init = true;
    }
    return s_spriteMaterial;
}

bool Sprite::Load(const jet::String &name, bool canLoadWithoutTGA) {
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name + ".bsprite");
    if (!s) return false;
    m_spriteFileName = name;
    m_canLoadWithoutTGA = canLoadWithoutTGA;
    Unload();
    s->Start();
    ASprite::Load(*s);
    jet::String tex;
    const jet::String &image = GetImageName(0);
    if (image.empty()) tex = jet::String::Format("sprite_%s.tga", name.c_str());
    else tex = jet::String::Format("sprite_%s", image.c_str());
    jet::video::TextureLoader *tl = jet::video::TextureLoader::GetInstance();
    m_paletteTextures.push_back(tl->Load(tex));
    jet::String alpha = tex + "__alpha";
    m_paletteTexturesAlpha.push_back(tl->CanLoad(alpha) ? tl->Load(alpha) : boost::shared_ptr<jet::video::Texture>());
    s->Finish();
    delete s;
    return true;
}

Sprite *Sprite::LoadSprite(const jet::String &name, bool canLoadWithoutTGA) {
    Sprite *sp = new Sprite();
    if (!sp->Load(name, canLoadWithoutTGA)) { delete sp; return 0; }
    return sp;
}

// _PaintPrepareRender @0x640e84: the palette's texture (and alpha) into the sprite material, the
// blending mode, the material into the painter, and the inverse texture size for the uvs
void Sprite::_PaintPrepareRender(jet::video::Painter &painter) {
    using namespace jet::video;
    Material &mat = m_tempMaterial ? *m_tempMaterial : GetMaterial();
    if (!mat.GetRenderPassCount()) return;
    if (mat.GetTextureLayerCount(0)) {
        TextureLayer &layer = mat.GetTextureLayer(0, 0);
        layer.SetTexture(GetTexture((unsigned)m_currentPalette));
        layer.SetAlphaTexture(GetAlphaTexture((unsigned)m_currentPalette));
    }
    BlendFormula::FactorDef::type src = BlendFormula::FactorDef::SrcAlpha, dst = BlendFormula::FactorDef::InvSrcAlpha;
    bool blend = true;
    switch (s_crtBlending) {
    case SpriteBlendingDef::Alpha: break;
    case SpriteBlendingDef::Additive: case SpriteBlendingDef::Additive2: dst = BlendFormula::FactorDef::One; break;
    case SpriteBlendingDef::Multiply: src = BlendFormula::FactorDef::Zero; dst = BlendFormula::FactorDef::SrcColor; break;
    default: blend = false; src = BlendFormula::FactorDef::Zero; dst = BlendFormula::FactorDef::Zero; break;
    }
    RenderState &rs = mat.GetRenderState(0);
    rs.SetCulling(false);
    rs.SetBlending(blend);
    rs.SetBlendFormula(BlendFormula(src, dst, src, dst));
    painter.SetMaterial(mat);

    if ((unsigned)m_currentPalette < m_imagesInvSize.size()) {
        m_cachedInvTexSize = m_imagesInvSize[m_currentPalette];
    } else {
        math::vec2<float> size(1, 1);
        if (boost::shared_ptr<Texture> t = GetTexture((unsigned)m_currentPalette)) {
            math::vec2<unsigned int> ts = t->GetSize();
            size = math::vec2<float>(ts.x > 1 ? (float)ts.x : 1.0f, ts.y > 1 ? (float)ts.y : 1.0f);
        }
        m_cachedInvTexSize = math::vec2<float>(1.0f / size.x, 1.0f / size.y);
    }
    m_cachedInvTexSize.x *= m_textureScale;
    m_cachedInvTexSize.y *= m_textureScale;
}

// @0x4bfdcc
void Sprite::_PaintRenderDone() {
    jet::video::Material &mat = m_tempMaterial ? *m_tempMaterial : GetMaterial();
    if (!mat.GetRenderPassCount() || !mat.GetTextureLayerCount(0)) return;
    jet::video::TextureLayer &layer = mat.GetTextureLayer(0, 0);
    layer.SetTexture(boost::shared_ptr<jet::video::Texture>());
    layer.SetAlphaTexture(boost::shared_ptr<jet::video::Texture>());
}

static math::mat3<float> Translation(const math::vec2<float> &p) {
    math::mat3<float> m;
    m.m[6] = p.x; m.m[7] = p.y;
    return m;
}

void Sprite::PaintAnim(jet::video::Painter &p, unsigned anim, int t, const math::mat3<float> &m) {
    _PaintPrepareRender(p);
    _PaintAnim(p, anim, t, m);
}
void Sprite::PaintAnim(jet::video::Painter &p, unsigned anim, int t, const math::vec2<float> &pos) { PaintAnim(p, anim, t, Translation(pos)); }
void Sprite::PaintAFrame(jet::video::Painter &p, unsigned anim, unsigned af, const math::mat3<float> &m) {
    _PaintPrepareRender(p);
    _PaintAFrame(p, anim, af, m);
}
void Sprite::PaintAFrame(jet::video::Painter &p, unsigned anim, unsigned af, const math::vec2<float> &pos) { PaintAFrame(p, anim, af, Translation(pos)); }
void Sprite::PaintFrame(jet::video::Painter &p, unsigned frame, const math::mat3<float> &m) {
    _PaintPrepareRender(p);
    _PaintFrame(p, frame, m);
}
void Sprite::PaintFrame(jet::video::Painter &p, unsigned frame, const math::vec2<float> &pos) { PaintFrame(p, frame, Translation(pos)); }
void Sprite::PaintFModule(jet::video::Painter &p, unsigned frame, unsigned fm, const math::mat3<float> &m) {
    _PaintPrepareRender(p);
    _PaintFModule(p, frame, fm, m);
}
void Sprite::PaintModule(jet::video::Painter &p, unsigned module, const math::mat3<float> &m) {
    _PaintPrepareRender(p);
    _PaintModule(p, module, m);
}
void Sprite::PaintModule(jet::video::Painter &p, unsigned module, const math::vec2<float> &pos) { PaintModule(p, module, Translation(pos)); }

// @0x640a7c
void Sprite::_PaintAnim(jet::video::Painter &p, unsigned anim, int t, const math::mat3<float> &m) {
    _PaintAFrame(p, anim, (unsigned)GetAFrame((int)anim, t), m);
}

// @0x640af4: the aframe's transform, its offset scaled by the position scale
void Sprite::_PaintAFrame(jet::video::Painter &p, unsigned anim, unsigned af, const math::mat3<float> &m) {
    const AFrameData &a = m_aFrames[m_animsFirstAFrameIndex[anim] + af];
    math::mat3<float> local = a.transform;
    local.m[6] *= m_posScale;
    local.m[7] *= m_posScale;
    _PaintFrame(p, a.frame, m * local);
}

// @0x640b90
void Sprite::_PaintFrame(jet::video::Painter &p, unsigned frame, const math::mat3<float> &m) {
    for (unsigned i = 0; i < m_framesNumberOfFModules[frame]; i++) _PaintFModule(p, frame, i, m);
}

// @0x640be0
void Sprite::_PaintFModule(jet::video::Painter &p, unsigned frame, unsigned fm, const math::mat3<float> &m) {
    const FModuleData &f = m_fmodules[m_framesFMStart[frame] + fm];
    if (m_moduleInfo[f.module].type == 0xfd) return;
    math::mat3<float> local = f.transform;
    local.m[6] *= m_posScale;
    local.m[7] *= m_posScale;
    if (f.flags & FLAG_HYPER_FRAME) _PaintFrame(p, (unsigned)f.module, m * local);
    else _PaintModule(p, (unsigned)f.module, m * local);
}

// @0x640ca8: one textured quad
void Sprite::_PaintModule(jet::video::Painter &p, unsigned module, const math::mat3<float> &m) {
    const ModuleInfo &mi = m_moduleInfo[module];
    const float w = mi.w * m_posScale, h = mi.h * m_posScale;
    math::vec2<float> c[4] = {
        math::vec2<float>(m.m[6], m.m[7]),
        math::vec2<float>(m.m[0] * w + m.m[6], m.m[1] * w + m.m[7]),
        math::vec2<float>(m.m[0] * w + m.m[3] * h + m.m[6], m.m[1] * w + m.m[4] * h + m.m[7]),
        math::vec2<float>(m.m[3] * h + m.m[6], m.m[4] * h + m.m[7]),
    };
    const float u0 = mi.x * m_cachedInvTexSize.x, u1 = (mi.x + mi.w) * m_cachedInvTexSize.x;
    const float v0 = mi.y * m_cachedInvTexSize.y, v1 = (mi.y + mi.h) * m_cachedInvTexSize.y;
    typedef jet::video::Painter::Vertex V;
    p.FillQuad(V(math::vec3<float>(c[0].x, c[0].y, 0), m_currentColor, math::vec2<float>(u0, v0)),
               V(math::vec3<float>(c[1].x, c[1].y, 0), m_currentColor, math::vec2<float>(u1, v0)),
               V(math::vec3<float>(c[2].x, c[2].y, 0), m_currentColor, math::vec2<float>(u1, v1)),
               V(math::vec3<float>(c[3].x, c[3].y, 0), m_currentColor, math::vec2<float>(u0, v1)));
}

void Sprite::UpdateProgressBarUniforms(unsigned module, float minAngle, float maxAngle) {
    const ModuleInfo &mi = GetModule((int)module);
    boost::shared_ptr<jet::video::Texture> tex = GetTexture(0);
    float w = tex ? (float)tex->GetSize().x : 1.0f;
    jet::video::Material &mat = GetMaterial();
    int idx = mat.FindUniformIdxByName(0, jet::String("size"));
    if (idx >= 0) mat.SetUniform(0, (unsigned)idx, jet::video::ShaderUniform(math::vec2<float>((float)mi.w / w, (float)mi.h / w)));
    idx = mat.FindUniformIdxByName(0, jet::String("initial"));
    if (idx >= 0) mat.SetUniform(0, (unsigned)idx, jet::video::ShaderUniform(math::vec2<float>((float)mi.x / w, (float)mi.y / w)));
    idx = mat.FindUniformIdxByName(0, jet::String("minAngle"));
    if (idx >= 0) mat.SetUniform(0, (unsigned)idx, jet::video::ShaderUniform(minAngle));
    idx = mat.FindUniformIdxByName(0, jet::String("maxAngle"));
    if (idx >= 0) mat.SetUniform(0, (unsigned)idx, jet::video::ShaderUniform(maxAngle));
}

bool Sprite::GetFrameLogicalRect(int frame, int idx, math::vec2<float> &pos, math::vec2<float> &size) const {
    if (!ASprite::GetFrameLogicalRect(frame, idx, pos, size)) return false;
    pos = math::vec2<float>(pos.x * m_posScale, pos.y * m_posScale);
    size = math::vec2<float>(size.x * m_posScale, size.y * m_posScale);
    return true;
}
