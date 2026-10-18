#include <cstdio>
#include <cstdlib>
// core/jet/src/video/gles20/gles20driver.cpp
#include "video/gles20/gles20driver.h"
#include "video/renderjob.h"
#include "video/posteffect.h"
#include "video/gles20/gles20shaderprogram.h"
#include "video/display.h"
#include "video/rendertechnique.h"
#include "video/renderpass.h"
#include "video/texturelayer.h"
#include "video/gles20/gles20rendertargetwrapper.h"
#include "video/colorcorrection.h"
#include <cstring>
#include <map>

namespace jet::video {

GLES20Driver::GLES20Driver()
    : m_display(0), m_betweenBeginEndRendering(false), m_isFogEnabled(false), m_fogColor(0.5f, 0.5f, 0.5f, 1),
      m_fogStart(0), m_fogEnd(1000), m_maxTextureUnits(8), m_maxLightCount(4),
      m_batchingVertexLimit(0xffff), m_flushCount(0), m_hasAppliedRenderState(false) {
}
GLES20Driver::~GLES20Driver() {}

bool GLES20Driver::Init(Display *display) {
    m_display = display;
    InitFeatures();
    gles::Interface gl;
    int units = 0;
    gl.iglGetIntegerv(0x8B4C /*GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS*/, &units);
    if (units > 0) m_maxTextureUnits = units;
    Driver::SetCurrent(this);
    return true;
}

void GLES20Driver::Shutdown(Display*) { m_display = 0; if (Driver::GetCurrent() == this) Driver::SetCurrent(0); }
Display *GLES20Driver::GetDisplay() { return m_display; }

// GetArchitectureString @0x6add50: the target name technique passes are selected by
jet::String GLES20Driver::GetArchitectureString() {
    return jet::String("GLES2.0");
}

void GLES20Driver::InitFeatures() {
    gles::Interface gl;
    const unsigned char *ext = gl.iglGetString(0x1F03 /*GL_EXTENSIONS*/);
    const char *e = ext ? reinterpret_cast<const char*>(ext) : "";
    if (strstr(e, "texture_npot") || strstr(e, "OES_texture_npot")) m_supportedFeatures.insert((int)Driver::FeatureDef::NPOTTextures);
    if (strstr(e, "depth_texture")) m_supportedFeatures.insert((int)Driver::FeatureDef::DepthTexture);
    if (strstr(e, "multisample")) m_supportedFeatures.insert((int)Driver::FeatureDef::MSAA);
    if (strstr(e, "vertex_array_object")) m_supportedFeatures.insert((int)Driver::FeatureDef::VertexArrayObject);
    if (strstr(e, "mapbuffer") || strstr(e, "map_buffer")) m_supportedFeatures.insert((int)Driver::FeatureDef::MapBuffer);
    if (strstr(e, "anisotropic")) m_supportedFeatures.insert((int)Driver::FeatureDef::Anisotropy);
    // GLES 3.0 has NPOT textures, depth textures, multisample renderbuffers and VAOs in core (no extension string needed)
    if (gles::ClientVersion() >= 3) {
        m_supportedFeatures.insert((int)Driver::FeatureDef::NPOTTextures);
        m_supportedFeatures.insert((int)Driver::FeatureDef::DepthTexture);
        m_supportedFeatures.insert((int)Driver::FeatureDef::MSAA);
        m_supportedFeatures.insert((int)Driver::FeatureDef::VertexArrayObject);
    }
}
bool GLES20Driver::IsFeatureSupported(safe_enum<Driver::FeatureDef,Driver::FeatureDef::type> f) {
    return m_supportedFeatures.find((int)(Driver::FeatureDef::type)f) != m_supportedFeatures.end();
}

void GLES20Driver::SetMaxLightCount(unsigned c) { m_maxLightCount = c; }
unsigned GLES20Driver::GetMaxLightCount() { return m_maxLightCount; }
unsigned GLES20Driver::GetMaxTextureUnits() { return m_maxTextureUnits; }
void GLES20Driver::SetBatchVertexLimit(unsigned l) { m_batchingVertexLimit = l; }

void GLES20Driver::SetCrtRenderTarget(const boost::shared_ptr<RenderTarget> &rt) {
    Driver::SetCrtRenderTarget(rt);
    if (rt) static_cast<GLES20RenderTargetWrapper*>(rt.get())->Activate();
}

bool GLES20Driver::BeginRendering() {
    if (!Driver::BeginRendering()) return false;
    m_betweenBeginEndRendering = true;
    m_hasAppliedRenderState = false;
    return true;
}
bool GLES20Driver::EndRendering() {
    gles::Interface gl;
    gl.iglFlush();
    m_betweenBeginEndRendering = false;
    return Driver::EndRendering();
}
void GLES20Driver::FinishAllRendering() { gles::Interface gl; gl.iglFinish(); }
void GLES20Driver::Flush() { gles::Interface gl; gl.iglFlush(); m_flushCount++; }
void GLES20Driver::SyncAll() { gles::Interface gl; gl.isyncAll(); }

void GLES20Driver::SetFog(bool v) { m_isFogEnabled = v; }
bool GLES20Driver::GetFog() { return m_isFogEnabled; }
void GLES20Driver::SetFogParams(const Color &c, float start, float end) { m_fogColor = c; m_fogStart = start; m_fogEnd = end; }
void GLES20Driver::GetFogParams(Color &c, float &start, float &end) { c = m_fogColor; start = m_fogStart; end = m_fogEnd; }

bool GLES20Driver::AddLight(const LightData &l) {
    if (m_lights.size() >= m_maxLightCount) return false;
    m_lights.push_back(l);
    return true;
}
void GLES20Driver::ResetLights() { m_lights.clear(); }

// Post effects (BeginPostEffect @0x6b1380, EndPostEffect @0x6b1610): the scene goes to the display's
// post-effect target 0; at the end each effect draws a fullscreen quad of its material, reading
// the previous target ("previous" samplers / rendertarget.ratio.previous) and writing the next,
// the last one into the target that was current at Begin.
boost::shared_ptr<RenderTarget> GLES20Driver::GetPostEffectRenderTarget(unsigned i) {
    boost::shared_ptr<RenderTarget> rt = m_display->GetPostEffectRenderTarget(i);
    if (!rt) {
        math::vec2<unsigned int> size = m_display->GetSize();
        rt = m_display->CreatePostEffectRenderTarget(size.x, size.y);
    }
    return rt;
}

boost::shared_ptr<RenderTarget> GLES20Driver::ResolveRenderTarget(unsigned idx) {
    if (idx == k_previousRenderTargetIdx) return m_previousRenderTarget;
    if (idx == k_defaultRenderTargetIdx) return m_display ? m_display->GetRenderTarget() : boost::shared_ptr<RenderTarget>();
    return idx < m_renderTargets.size() ? m_renderTargets[idx] : boost::shared_ptr<RenderTarget>();
}

void GLES20Driver::BeginPostEffect(PostEffect *e, safe_enum<PostFXAADef,PostFXAADef::type> aa) {
    std::vector<PostEffect*> v(1, e);
    BeginPostEffect(v, aa);
}

void GLES20Driver::BeginPostEffect(const std::vector<PostEffect*> &effects, safe_enum<PostFXAADef,PostFXAADef::type>) {
    m_postEffects.clear();
    for (unsigned i = 0; i < effects.size(); i++)
        if (effects[i] && effects[i]->GetRenderTechnique()) m_postEffects.push_back(effects[i]);
    if (m_postEffects.empty()) return;
    m_postEffectFinalTarget = GetCrtRenderTarget();
    boost::shared_ptr<RenderTarget> rt = GetPostEffectRenderTarget(0);
    rt->SetClearColor(m_postEffectFinalTarget ? m_postEffectFinalTarget->GetClearColor() : math::vec4<float>(0, 0, 0, 1));
    rt->SetClearColorBuffer(true);
    rt->SetClearDepthBuffer(true);
    SetCrtRenderTarget(rt);
    static_cast<GLES20RenderTargetWrapper*>(rt.get())->Clear();
}

void GLES20Driver::EndPostEffect(const math::mat4<float> &transform) {
    if (m_postEffects.empty()) return;
    if (!m_postEffectQuad) {
        // clip-space quad, uv (0,0) at the bottom left
        typedef Geometry::SemanticDef Sem;
        m_postEffectQuad = Geometry::New();
        m_postEffectQuad->SetVertexCount(4);
        m_postEffectQuad->SetIndexCount(6);
        m_postEffectQuad->SetTopology(Geometry::TopologyDef::Triangles);
        m_postEffectQuad->AddAttribute(jet::String("positions"), Sem::Position, Geometry::UsageDef::Static, Geometry::TypeDef::Float, 3);
        m_postEffectQuad->AddAttribute(jet::String("uv0"), Sem::TexCoord0, Geometry::UsageDef::Static, Geometry::TypeDef::Float, 2);
        m_postEffectQuad->Compile();
        static const float pos[] = { -1, -1, 0, 1, -1, 0, 1, 1, 0, -1, 1, 0 };
        static const float uv[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
        static const unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };
        for (unsigned a = 0; a < 2; a++) {
            unsigned char *d = m_postEffectQuad->MapAttributeU8Write(a);
            unsigned stride = m_postEffectQuad->GetAttributeStride(a);
            for (unsigned v = 0; v < 4; v++) memcpy(d + v * stride, a ? &uv[v * 2] : &pos[v * 3], a ? 8 : 12);
            m_postEffectQuad->UnmapAttribute(a);
        }
        memcpy(m_postEffectQuad->MapIndicesWrite(), idx, sizeof(idx));
        m_postEffectQuad->UnmapAll();
    }
    for (unsigned i = 0; i < m_postEffects.size(); i++) {
        const bool last = i + 1 == m_postEffects.size();
        m_previousRenderTarget = GetPostEffectRenderTarget(i & 1);
        boost::shared_ptr<RenderTarget> out = last ? m_postEffectFinalTarget : GetPostEffectRenderTarget((i + 1) & 1);
        SetCrtRenderTarget(out);
        RenderJob job;
        job.SetGeometry(m_postEffectQuad);
        job.SetMaterial(&m_postEffects[i]->GetMaterial());
        job.SetTransform(transform, true);
        DrawPrimitives(job);
    }
    m_postEffects.clear();
    m_previousRenderTarget.reset();
    m_postEffectFinalTarget.reset();
}

// LinkUniform @0x6b6bf8: engine values a shader uniform can be bound to by its "val". The link
// type is kept as the uniform's server data (flag bit 2 = linked) and replaces the declared type.
// "$name" binds a global uniform (link 0xb0 + global index), "rendertarget.ratio.<rt>" /
// "rendertarget.size.<rt>" a render target's viewport ratio / size.
namespace {
struct UniformLink { const char *name; unsigned short link; unsigned char type; };
typedef ShaderUniform::TypeDef UT;
static const UniformLink s_uniformLinks[] = {
    { "matrix.worldViewProj", 28, UT::Mat4 }, { "matrix.worldViewProjI", 30, UT::Mat4 },
    { "matrix.worldViewProjT", 32, UT::Mat4 }, { "matrix.worldViewProjIT", 34, UT::Mat4 },
    { "matrix.worldView", 36, UT::Mat4 }, { "matrix.worldViewI", 38, UT::Mat4 },
    { "matrix.worldViewT", 40, UT::Mat4 }, { "matrix.worldViewIT", 42, UT::Mat4 },
    { "matrix.viewProj", 44, UT::Mat4 }, { "matrix.viewProjI", 46, UT::Mat4 },
    { "matrix.world", 2, UT::Mat4 }, { "matrix.world.mat3", 2, UT::Mat3 },
    { "matrix.world.3vec4", 2, UT::Mat43 }, { "matrix.world.mat43", 2, UT::Mat43 },
    { "matrix.worldI", 15, UT::Mat4 }, { "matrix.worldT", 16, UT::Mat4 },
    { "matrix.worldIT", 17, UT::Mat4 }, { "matrix.worldIT.mat3", 17, UT::Mat3 },
    { "matrix.worldIT.3vec4", 17, UT::Mat43 }, { "matrix.worldIT.mat43", 17, UT::Mat43 },
    { "matrix.view", 3, UT::Mat4 }, { "matrix.viewI", 18, UT::Mat4 }, { "matrix.viewT", 20, UT::Mat4 },
    { "matrix.viewIT", 22, UT::Mat4 }, { "matrix.proj", 5, UT::Mat4 }, { "matrix.projI", 26, UT::Mat4 },
    { "matrix.texture0", 7, UT::Mat4 }, { "matrix.texture0.2vec3", 7, UT::Mat32 }, { "matrix.texture0.mat32", 7, UT::Mat32 },
    { "matrix.texture1", 8, UT::Mat4 }, { "matrix.texture1.2vec3", 8, UT::Mat32 }, { "matrix.texture1.mat32", 8, UT::Mat32 },
    { "matrix.texture2", 9, UT::Mat4 }, { "matrix.texture2.2vec3", 9, UT::Mat32 }, { "matrix.texture2.mat32", 9, UT::Mat32 },
    { "matrix.texture3", 10, UT::Mat4 }, { "matrix.texture3.2vec3", 10, UT::Mat32 }, { "matrix.texture3.mat32", 10, UT::Mat32 },
    { "matrix.texture4", 11, UT::Mat4 }, { "matrix.texture4.2vec3", 11, UT::Mat32 }, { "matrix.texture4.mat32", 11, UT::Mat32 },
    { "matrix.texture5", 12, UT::Mat4 }, { "matrix.texture5.2vec3", 12, UT::Mat32 }, { "matrix.texture5.mat32", 12, UT::Mat32 },
    { "matrix.texture6", 13, UT::Mat4 }, { "matrix.texture6.2vec3", 13, UT::Mat32 }, { "matrix.texture6.mat32", 13, UT::Mat32 },
    { "matrix.texture7", 14, UT::Mat4 }, { "matrix.texture7.2vec3", 14, UT::Mat32 }, { "matrix.texture7.mat32", 14, UT::Mat32 },
    { "camera.position", 50, UT::Vec3 }, { "camera.positionOS", 51, UT::Vec3 }, { "camera.front", 52, UT::Vec3 },
    { "camera.frontOS", 53, UT::Vec3 }, { "camera.right", 54, UT::Vec3 }, { "camera.rightOS", 55, UT::Vec3 },
    { "camera.up", 56, UT::Vec3 }, { "camera.upOS", 57, UT::Vec3 },
    { "mesh.colorCorrection_Minion", 91, UT::Mat4 }, { "mesh.colorCorrection_Background", 92, UT::Mat4 },
    { "mesh.colorCorrection_Banana", 93, UT::Mat4 }, { "mesh.center", 86, UT::Vec3 },
    { "dlight.count", 60, UT::Int }, { "dlight.direction", 61, UT::Vec3 }, { "dlight.directionOS", 62, UT::Vec3 },
    { "dlight.ambient", 63, UT::Vec3 }, { "dlight.diffuse", 64, UT::Vec3 }, { "dlight.specular", 65, UT::Vec3 },
    { "plight.count", 66, UT::Int }, { "plight.position", 67, UT::Vec3 }, { "plight.positionOS", 68, UT::Vec3 },
    { "plight.ambient", 69, UT::Vec3 }, { "plight.diffuse", 70, UT::Vec3 }, { "plight.specular", 71, UT::Vec3 },
    { "fog.enabled", 74, UT::Int }, { "fog.color", 75, UT::Vec4 }, { "fog.start", 76, UT::Float },
    { "fog.end", 77, UT::Float }, { "fog.range", 78, UT::Float }, { "fog.rangeI", 79, UT::Float },
    { "material.textureCount", 82, UT::Int }, { "material.color", 83, UT::Vec4 },
    { "mesh.lod", 87, UT::Int }, { "mesh.lodColor", 88, UT::Vec4 }, { "mesh.nodetransforms", 89, UT::Vec4 },
    { "driver.viewportSize", 97, UT::Vec4 }, { "matrix.proj2d", 98, UT::Mat4 },
    { "matrix.proj2d.2vec3", 98, UT::Mat32 }, { "matrix.proj2d.mat32", 98, UT::Mat32 },
    { "driver.frame", 99, UT::Int }, { "driver.time", 100, UT::Float }, { "driver.fractime", 101, UT::Float },
    { "driver.wraptime", 102, UT::Float },
};
}

bool GLES20Driver::LinkUniform(ShaderUniform &u, const jet::String &val) {
    if (val.empty()) return false;
    const char *c = val.c_str();
    unsigned link = 0, count = 1;
    safe_enum<ShaderUniform::TypeDef,unsigned char> type;
    if (c[0] == '$') {
        jet::String name(c + 1);
        int idx = FindGlobalUniformIdxByName(name);
        if (idx < 0) {
            if (!u.IsValid()) { printf("Please specify the type of global uniform '%s'\n", c); return false; }
            idx = (int)SetGlobalUniform(name, ShaderUniform(u.GetType(), (unsigned short)u.GetCount()));
        }
        const ShaderUniform &g = GetGlobalUniform((unsigned)idx, m_globalUniformCrtSetIdx);
        link = 0xb0 + idx;
        type = g.GetType();
        count = g.GetCount();
    } else if (!strncmp(c, "rendertarget.ratio.", 19) || !strncmp(c, "rendertarget.size.", 18)) {
        bool ratio = c[13] == 'r';
        int rt = FindRenderTargetIdxByName(jet::String(c + (ratio ? 19 : 18)));
        if (rt < 0) return false;
        link = (ratio ? k_linkRenderTargetRatio : k_linkRenderTargetSize) + rt;
        type = safe_enum<ShaderUniform::TypeDef,unsigned char>(ShaderUniform::TypeDef::Vec2);
    } else {
        const UniformLink *l = 0;
        for (unsigned i = 0; i < sizeof(s_uniformLinks) / sizeof(s_uniformLinks[0]) && !l; i++)
            if (val.Equals(s_uniformLinks[i].name)) l = &s_uniformLinks[i];
        if (!l) return false;
        link = l->link;
        type = safe_enum<ShaderUniform::TypeDef,unsigned char>((ShaderUniform::TypeDef::type)l->type);
    }
    u = ShaderUniform(type, (unsigned short)count);
    u.SetServerData(link);
    return true;
}
bool GLES20Driver::LinkSampler(ShaderSampler&, const jet::String&) { return false; }
boost::shared_ptr<RenderTarget> GLES20Driver::CreateRenderTarget() { return RenderTarget::New(); }
ShaderProgram *GLES20Driver::CreateShaderProgram(RenderTechnique *t) { return ShaderProgram::New(t); }
boost::shared_ptr<Texture> GLES20Driver::CreateTexture() { return Texture::New(); }
boost::shared_ptr<Geometry> GLES20Driver::CreateGeometry() { return Geometry::New(); }

// After UploadRenderState @0x6b8440; GL tables from 0xc95004 (compare), 0xc95024 (stencil op),
// 0xc95044 (blend factor), 0xc95070 (cull face).
void GLES20Driver::ApplyRenderState(const RenderState &rs) {
    gles::Interface gl;
    if (!getenv("JET_NORSCACHE") && m_hasAppliedRenderState && rs == m_appliedRenderState) return;
    static const unsigned funcs[] = { 0x0201 /*LESS*/, 0x0203 /*LEQUAL*/, 0x0202 /*EQUAL*/, 0x0206 /*GEQUAL*/,
                                      0x0204 /*GREATER*/, 0x0205 /*NOTEQUAL*/, 0x0207 /*ALWAYS*/, 0x0200 /*NEVER*/ };
    static const unsigned stencilOps[] = { 0x1e00 /*KEEP*/, 0 /*ZERO*/, 0x1e01 /*REPLACE*/, 0x150a /*INVERT*/,
                                           0x1e02 /*INCR*/, 0x1e03 /*DECR*/, 0x8507 /*INCR_WRAP*/, 0x8508 /*DECR_WRAP*/ };
    static const unsigned factors[] = { 0, 1, 0x0300 /*SRC_COLOR*/, 0x0301, 0x0306 /*DST_COLOR*/, 0x0307,
                                        0x0302 /*SRC_ALPHA*/, 0x0303, 0x0304 /*DST_ALPHA*/, 0x0305, 0x0308 /*SRC_ALPHA_SATURATE*/ };
    static const unsigned faces[] = { 0x0405 /*BACK*/, 0x0404 /*FRONT*/, 0x0408 /*FRONT_AND_BACK*/ };
#define JET_U8(e) ((unsigned char)(e))
    if (rs.GetStencilTest()) {
        gl.iglEnable(0x0B90 /*GL_STENCIL_TEST*/);
        gl.iglStencilFunc(funcs[JET_U8((RenderState::CompareFuncDef::type)rs.GetStencilFunc())], rs.GetStencilRef(), rs.GetStencilMask());
        gl.iglStencilOpSeparate(0x0404, stencilOps[JET_U8((RenderState::StencilOpDef::type)rs.GetStencilFrontSFailOp())],
                                stencilOps[JET_U8((RenderState::StencilOpDef::type)rs.GetStencilFrontZFailOp())],
                                stencilOps[JET_U8((RenderState::StencilOpDef::type)rs.GetStencilFrontPassOp())]);
        gl.iglStencilOpSeparate(0x0405, stencilOps[JET_U8((RenderState::StencilOpDef::type)rs.GetStencilBackSFailOp())],
                                stencilOps[JET_U8((RenderState::StencilOpDef::type)rs.GetStencilBackZFailOp())],
                                stencilOps[JET_U8((RenderState::StencilOpDef::type)rs.GetStencilBackPassOp())]);
    } else gl.iglDisable(0x0B90);
    rs.GetDepthTest() ? gl.iglEnable(0x0B71 /*GL_DEPTH_TEST*/) : gl.iglDisable(0x0B71);
    gl.iglDepthMask(rs.GetDepthWrite());
    gl.iglDepthFunc(funcs[JET_U8((RenderState::CompareFuncDef::type)rs.GetDepthFunc())]);
    unsigned char m = rs.GetColorMask();
    gl.iglColorMask(m & 1, (m >> 1) & 1, (m >> 2) & 1, (m >> 3) & 1);
    if (rs.GetCulling()) {
        gl.iglEnable(0x0B44 /*GL_CULL_FACE*/);
        gl.iglCullFace(faces[JET_U8((RenderState::FaceDef::type)rs.GetCullFace())]);
    } else gl.iglDisable(0x0B44);
    if (rs.GetPolygonOffset()) {
        gl.iglEnable(0x8037 /*GL_POLYGON_OFFSET_FILL*/);
        gl.iglPolygonOffset(rs.GetPolygonOffsetFactor(), rs.GetPolygonOffsetUnits());
    } else gl.iglDisable(0x8037);
    if (rs.GetBlending()) {
        gl.iglEnable(0x0BE2 /*GL_BLEND*/);
        const BlendFormula &bf = rs.GetBlendFormula();
        gl.iglBlendFuncSeparate(factors[JET_U8((BlendFormula::FactorDef::type)bf.GetSrcRGB())],
                                 factors[JET_U8((BlendFormula::FactorDef::type)bf.GetDstRGB())],
                                 factors[JET_U8((BlendFormula::FactorDef::type)bf.GetSrcAlpha())],
                                 factors[JET_U8((BlendFormula::FactorDef::type)bf.GetDstAlpha())]);
        gl.iglBlendEquation(rs.IsBlendSubtract() ? 0x800A /*FUNC_SUBTRACT*/ : 0x8006 /*FUNC_ADD*/);
    } else gl.iglDisable(0x0BE2);
#undef JET_U8
    m_appliedRenderState = rs;
    m_hasAppliedRenderState = true;
}

// After UploadAutoUniforms @0x6b4758 (the texture matrices come from the material layers' transform): each linked uniform of the pass gets its engine value by
// link type (see LinkUniform). Matrices are column-major, uploaded untransposed; shaders compute
// matrix * vector, so worldViewProj = proj * view * world.
static math::mat4<float> Inverse(const math::mat4<float> &m) { math::mat4<float> r(m); r.invert(); return r; }
static math::mat4<float> Transpose(const math::mat4<float> &m) { math::mat4<float> r(m); r.transpose(); return r; }
static math::vec3<float> XformPoint(const math::mat4<float> &m, const math::vec3<float> &v) {
    return math::vec3<float>(m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12],
                             m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13],
                             m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14]);
}
static math::vec3<float> XformDir(const math::mat4<float> &m, const math::vec3<float> &v) {
    return math::vec3<float>(m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z,
                             m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z,
                             m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z);
}

int GLES20Driver::UniformLocation(gles::Interface &gl, unsigned program, const jet::String &name) {
    std::pair<unsigned, jet::String> key(program, name);
    std::map<std::pair<unsigned, jet::String>, int>::iterator it = m_uniformLocations.find(key);
    if (it != m_uniformLocations.end()) return it->second;
    int loc = gl.iglGetUniformLocation(program, name.c_str());
    m_uniformLocations[key] = loc;
    return loc;
}

void GLES20Driver::UploadAutoUniforms(gles::Interface &gl, GLES20ShaderProgramFlavor &flavor, RenderPass &pass, const RenderJob &job, const Material &mat) {
    const Driver::CameraParams &cam = GetCrtCameraParams();
    const math::mat4<float> &world = job.GetTransform();
    const unsigned prog = flavor.GetGLId();
    ShaderData &autoData = pass.GetAutoShaderData();
    for (unsigned i = 0; i < autoData.GetUniformCount(); i++) {
        const ShaderUniform &u = autoData.GetUniform(i);
        if (!u.HasServerData()) continue;
        int loc = UniformLocation(gl, prog, autoData.GetUniformName(i));
        if (loc < 0) continue;
        const unsigned link = u.GetServerData();
        math::mat4<float> m;
        bool isMatrix = true;
        switch (link) {
        case 2: m = world; break;
        case 3: m = cam.view; break;
        case 5: m = cam.projection; break;
        case 15: m = Inverse(world); break;
        case 16: m = Transpose(world); break;
        case 17: m = Transpose(Inverse(world)); break;
        case 18: m = Inverse(cam.view); break;
        case 20: m = Transpose(cam.view); break;
        case 22: m = Transpose(Inverse(cam.view)); break;
        case 26: m = Inverse(cam.projection); break;
        case 28: m = cam.projection * cam.view * world; break;
        case 30: m = Inverse(cam.projection * cam.view * world); break;
        case 32: m = Transpose(cam.projection * cam.view * world); break;
        case 34: m = Transpose(Inverse(cam.projection * cam.view * world)); break;
        case 36: m = cam.view * world; break;
        case 38: m = Inverse(cam.view * world); break;
        case 40: m = Transpose(cam.view * world); break;
        case 42: m = Transpose(Inverse(cam.view * world)); break;
        case 44: m = cam.projection * cam.view; break;
        case 46: m = Inverse(cam.projection * cam.view); break;
        case 98: {  // ComputeProjection2D @0x6b0c98: pixels (top-left origin) of the current target to clip space
            const boost::shared_ptr<RenderTarget> &rt = GetCrtRenderTarget();
            math::vec2<unsigned int> size = rt ? rt->GetSize() : m_display->GetSize();
            m.m[0] = 2.0f / (float)size.x; m.m[5] = -2.0f / (float)size.y; m.m[10] = -0.001f;
            m.m[12] = -1.0f; m.m[13] = 1.0f;
            break;
        }
        default:
            isMatrix = link >= 7 && link <= 14;
            if (isMatrix && link - 7 < mat.GetTextureLayerCount(0)) {  // texture matrices: the layer's transform (Model::_AnimateMaterials, flipbook / scrolling uv), else identity
                TextureLayer layer = mat.GetTextureLayer(0, link - 7);
                if (layer.HasTransform()) m = layer.GetTransform();
            }
            break;
        }
        if (isMatrix) {
            switch ((ShaderUniform::TypeDef::type)u.GetType()) {
            case ShaderUniform::TypeDef::Mat3: {
                float m3[9] = { m.m[0], m.m[1], m.m[2], m.m[4], m.m[5], m.m[6], m.m[8], m.m[9], m.m[10] };
                gl.iglUniformMatrix3fv(loc, 1, 0, m3);
                break;
            }
            case ShaderUniform::TypeDef::Mat43: {  // three rows
                float r[12] = { m.m[0], m.m[4], m.m[8], m.m[12], m.m[1], m.m[5], m.m[9], m.m[13], m.m[2], m.m[6], m.m[10], m.m[14] };
                gl.iglUniform4fv(loc, 3, r);
                break;
            }
            case ShaderUniform::TypeDef::Mat32: {
                float r[6] = { m.m[0], m.m[4], m.m[12], m.m[1], m.m[5], m.m[13] };
                gl.iglUniform3fv(loc, 2, r);
                break;
            }
            default: gl.iglUniformMatrix4fv(loc, 1, 0, m.m); break;
            }
            continue;
        }
        const math::mat4<float> invWorld = link == 51 || link == 53 || link == 55 || link == 57 || link == 62 || link == 68 ? Inverse(world) : math::mat4<float>();
        math::vec3<float> v;
        switch (link) {
        case 50: gl.iglUniform3fv(loc, 1, &cam.position.x); break;
        case 51: v = XformPoint(invWorld, cam.position); gl.iglUniform3fv(loc, 1, &v.x); break;
        case 52: gl.iglUniform3fv(loc, 1, &cam.front.x); break;
        case 53: v = XformDir(invWorld, cam.front); gl.iglUniform3fv(loc, 1, &v.x); break;
        case 54: gl.iglUniform3fv(loc, 1, &cam.right.x); break;
        case 55: v = XformDir(invWorld, cam.right); gl.iglUniform3fv(loc, 1, &v.x); break;
        case 56: gl.iglUniform3fv(loc, 1, &cam.up.x); break;
        case 57: v = XformDir(invWorld, cam.up); gl.iglUniform3fv(loc, 1, &v.x); break;
        case 60: case 66: {  // light counts
            int n = 0;
            for (unsigned l = 0; l < m_lights.size(); l++)
                if ((m_lights[l].GetType() == LightData::TypeDef::Directional) == (link == 60)) ++n;
            gl.iglUniform1i(loc, n);
            break;
        }
        case 61: case 62: case 63: case 64: case 65:
        case 67: case 68: case 69: case 70: case 71: {
            // arrays over the directional (61..65) or point (67..71) lights: direction/position
            // (world or object space), ambient, diffuse, specular
            const bool directional = link < 66;
            const unsigned what = directional ? link - 61 : link - 67;
            std::vector<float> v;
            math::mat4<float> invWorld = Inverse(world);
            for (unsigned l = 0; l < m_lights.size(); l++) {
                LightData &ld = m_lights[l];
                if ((ld.GetType() == LightData::TypeDef::Directional) != directional) continue;
                math::vec3<float> c;
                switch (what) {
                case 0: c = directional ? ld.GetDirection() : ld.GetPosition(); break;
                case 1: c = directional ? XformDir(invWorld, ld.GetDirection()) : XformPoint(invWorld, ld.GetPosition()); break;
                case 2: c = ld.GetAmbientColor(); break;
                case 3: c = ld.GetDiffuseColor(); break;
                default: c = ld.GetSpecularColor(); break;
                }
                v.push_back(c.x); v.push_back(c.y); v.push_back(c.z);
            }
            if (!v.empty()) gl.iglUniform3fv(loc, (int)(v.size() / 3), &v[0]);
            break;
        }
        case 74: gl.iglUniform1i(loc, m_isFogEnabled ? 1 : 0); break;
        case 75: { float c[4] = { m_fogColor.r, m_fogColor.g, m_fogColor.b, m_fogColor.a }; gl.iglUniform4fv(loc, 1, c); break; }
        case 76: gl.iglUniform1f(loc, m_fogStart); break;
        case 77: gl.iglUniform1f(loc, m_fogEnd); break;
        case 78: gl.iglUniform1f(loc, m_fogEnd - m_fogStart); break;
        case 79: gl.iglUniform1f(loc, m_fogEnd != m_fogStart ? 1.0f / (m_fogEnd - m_fogStart) : 0.0f); break;
        case 82: gl.iglUniform1i(loc, (int)mat.GetTextureLayerCount(0)); break;
        case 83: { unsigned c = mat.GetColor(); float f[4] = { (c & 0xff) / 255.0f, ((c >> 8) & 0xff) / 255.0f, ((c >> 16) & 0xff) / 255.0f, (c >> 24) / 255.0f };
                   gl.iglUniform4fv(loc, 1, f); break; }
        case 86: v = job.GetCenter(); gl.iglUniform3fv(loc, 1, &v.x); break;
        case 87: gl.iglUniform1i(loc, 0); break;  // debug lod colouring off
        case 88: { float f[4] = { 0, 0, 0, 0 }; gl.iglUniform4fv(loc, 1, f); break; }
        // @0x6b5b0c: the grading of the minions is the first profile's, the bananas' the sixth's
        case 91: gl.iglUniformMatrix4fv(loc, 1, 0, g_ColorCorrectionMatrix[0].m); break;
        // @0x6b5b48: the location's. The original also mixes the last and the active profile by g_TransitionTime
        // into a local, but uploads the active profile's matrix (@0x6b5d00)
        case 92: gl.iglUniformMatrix4fv(loc, 1, 0, g_ColorCorrectionMatrix[g_ActiveColorProfile].m); break;
        case 93: gl.iglUniformMatrix4fv(loc, 1, 0, g_ColorCorrectionMatrix[5].m); break;  // @0x6b5acc
        case 99: gl.iglUniform1i(loc, (int)m_frameIdx); break;
        case 100: gl.iglUniform1f(loc, m_driverTime); break;
        case 101: gl.iglUniform1f(loc, m_driverFracTime); break;
        case 102: gl.iglUniform1f(loc, m_driverWrapTime); break;
        case 89:
            if (job.GetNodeTransformCount()) {
                std::vector<float> rows(job.GetNodeTransformCount() * 12);
                for (unsigned n = 0; n < job.GetNodeTransformCount(); n++) {
                    const math::mat4<float> &t = job.GetNodeTransform(n);
                    float *r = &rows[n * 12];
                    r[0] = t.m[0]; r[1] = t.m[4]; r[2] = t.m[8]; r[3] = t.m[12];
                    r[4] = t.m[1]; r[5] = t.m[5]; r[6] = t.m[9]; r[7] = t.m[13];
                    r[8] = t.m[2]; r[9] = t.m[6]; r[10] = t.m[10]; r[11] = t.m[14];
                }
                gl.iglUniform4fv(loc, (int)job.GetNodeTransformCount() * 3, &rows[0]);
            }
            break;
        default:
            if (link >= k_linkRenderTargetRatio && link < k_linkRenderTargetSize + 0x1000) {
                // viewport over texture size (<1 when targets are rounded up to powers of two)
                boost::shared_ptr<RenderTarget> rt = ResolveRenderTarget(link & 0xfff);
                float v[2] = { 1, 1 };
                if (rt) {
                    math::vec2<unsigned int> size = rt->GetSize();
                    if (link >= k_linkRenderTargetSize) { v[0] = (float)size.x; v[1] = (float)size.y; }
                    else if (rt->GetColorTexture()) {
                        math::vec2<unsigned int> ts = rt->GetColorTexture()->GetSize();
                        if (ts.x && ts.y) { v[0] = (float)size.x / ts.x; v[1] = (float)size.y / ts.y; }
                    }
                }
                gl.iglUniform2fv(loc, 1, v);
            } else if (link >= 0xb0 && link < k_linkRenderTargetRatio) {
                const ShaderUniform &g = GetGlobalUniform(link - 0xb0, m_globalUniformCrtSetIdx);
                GLES20ShaderProgram::UploadUniform(gl, loc, g);
            }
            break;
        }
    }
}

void GLES20Driver::ApplyMaterial(gles::Interface &gl, GLES20ShaderProgramFlavor &flavor, const Material &mat, unsigned pass, const RenderJob&) {
    for (unsigned u = 0; u < mat.GetUniformCount(pass); u++) {
        int loc = gl.iglGetUniformLocation(flavor.GetGLId(), mat.GetUniformName(pass, u).c_str());
        if (loc < 0) continue;
        ShaderUniform uniform = mat.GetUniform(pass, u);
        switch ((ShaderUniform::TypeDef::type)uniform.GetType()) {
        case ShaderUniform::TypeDef::Int: gl.iglUniform1i(loc, uniform.GetInt()); break;
        case ShaderUniform::TypeDef::Float: gl.iglUniform1f(loc, uniform.GetFloat()); break;
        case ShaderUniform::TypeDef::Vec2: gl.iglUniform2fv(loc, 1, &uniform.GetVec2().x); break;
        case ShaderUniform::TypeDef::Vec3: gl.iglUniform3fv(loc, 1, &uniform.GetVec3().x); break;
        case ShaderUniform::TypeDef::Vec4: gl.iglUniform4fv(loc, 1, &uniform.GetVec4().x); break;
        default: break;
        }
    }
    // Samplers: layer t on unit t with its sampler uniform set to t; separate alpha textures
    // ("<sampler>_a_l_p_h_a", see PreprocessAlphaSamplers) follow on the next units.
    const unsigned layers = mat.GetTextureLayerCount(pass);
    unsigned unit = layers;
    for (unsigned t = 0; t < layers; t++) {
        TextureLayer layer = mat.GetTextureLayer(pass, t);
        const jet::String &name = mat.GetTextureLayerName(pass, t);
        int loc = UniformLocation(gl, flavor.GetGLId(), name);
        if (!layer.GetTexture()) continue;
        GLES20Texture *tex = static_cast<GLES20Texture*>(layer.GetTexture().get());
        tex->SetWrapS(layer.GetWrapS());
        tex->SetWrapT(layer.GetWrapT());
        tex->SetFiltering(layer.GetFilter());
        tex->SetMipMap(layer.GetMipmap());
        tex->Bind(gl, t);
        if (loc >= 0) gl.iglUniform1i(loc, (int)t);
        int aloc = UniformLocation(gl, flavor.GetGLId(), name + "_a_l_p_h_a");
        if (aloc < 0) continue;
        if (!layer.GetAlphaTexture()) {
            // no "__alpha" companion (the colour texture carries its own alpha): sample opaque white
            if (!m_whiteAlphaTexture) {
                const unsigned char white[4] = { 255, 255, 255, 255 };
                gl.iglGenTextures(1, &m_whiteAlphaTexture);
                gl.iglActiveTexture(0x84C0 /*GL_TEXTURE0*/ + unit);
                gl.iglBindTexture(0x0DE1, m_whiteAlphaTexture);
                gl.iglTexImage2D(0x0DE1, 0, 0x1908, 1, 1, 0, 0x1908, 0x1401, white);
                gl.iglTexParameteri(0x0DE1, 0x2801, 0x2600);
                gl.iglTexParameteri(0x0DE1, 0x2800, 0x2600);
            }
            gl.iglActiveTexture(0x84C0 /*GL_TEXTURE0*/ + unit);
            gl.iglBindTexture(0x0DE1, m_whiteAlphaTexture);
            gl.iglUniform1i(aloc, (int)unit);
            ++unit;
            continue;
        }
        GLES20Texture *atex = static_cast<GLES20Texture*>(layer.GetAlphaTexture().get());
        atex->SetWrapS(layer.GetWrapS());
        atex->SetWrapT(layer.GetWrapT());
        atex->SetFiltering(layer.GetFilter());
        atex->SetMipMap(layer.GetMipmap());
        atex->Bind(gl, unit);
        gl.iglUniform1i(aloc, (int)unit);
        ++unit;
    }
    // Samplers bound by the technique (val="previous" or a render target name) read that
    // target's colour texture
    const ShaderData &autoData = mat.GetRenderTechnique()->GetRenderPass(pass).GetAutoShaderData();
    for (unsigned t = 0; t < autoData.GetSamplerCount(); t++) {
        const ShaderSampler &samp = autoData.GetSampler(t);
        int loc = UniformLocation(gl, flavor.GetGLId(), autoData.GetSamplerName(t));
        if (loc < 0) continue;
        const jet::String binding = samp.GetBinding();
        boost::shared_ptr<Texture> texture;
        if (binding.size() > 1 && binding.c_str()[0] == '$') {
            // "$name": a driver global texture (e.g. LightDef's "LightPalette")
            texture = FindGlobalTextureByName(jet::String(binding.c_str() + 1));
        } else {
            int idx = FindRenderTargetIdxByName(binding);
            boost::shared_ptr<RenderTarget> rt = idx >= 0 ? ResolveRenderTarget((unsigned)idx) : boost::shared_ptr<RenderTarget>();
            if (rt) texture = rt->GetColorTexture();
        }
        if (!texture) continue;
        GLES20Texture *tex = static_cast<GLES20Texture*>(texture.get());
        tex->SetWrapS(samp.GetWrapS());
        tex->SetWrapT(samp.GetWrapT());
        tex->SetFiltering(samp.GetFilter());
        tex->SetMipMap(binding.c_str()[0] == '$' && tex->HasMipMaps());
        tex->Bind(gl, unit);
        gl.iglUniform1i(loc, (int)unit);
        ++unit;
    }
}

bool GLES20Driver::DrawPrimitives(const RenderJob &job) {
    if (!job.IsValid() || !m_betweenBeginEndRendering) return false;
    const Material &mat = *job.GetMaterial();
    boost::shared_ptr<RenderTechnique> tech = mat.GetRenderTechnique();
    if (!tech) return false;
    gles::Interface gl;
    for (unsigned p = 0; p < tech->GetRenderPassCount(); p++) {
        RenderPass &pass = tech->GetRenderPass(p);
        if ((pass.GetTag() != 0) && (GetTagMask() & (1u << (pass.GetTag() & 0xff))) == 0) continue;
        ApplyRenderState(mat.GetRenderState(p));
        ShaderProgram *prog = pass.GetShaderProgram();
        if (!prog) { pass.Link(); prog = pass.GetShaderProgram(); }
        if (!prog) continue;
        GLES20ShaderProgramFlavor *flavor = static_cast<GLES20ShaderProgramFlavor*>(
            prog->GetFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>(
                                 (ShaderProgramFlavor::TypeDef::type)(job.GetSkinWeightCount() > 4 ? 4 : job.GetSkinWeightCount())),
                             mat, p, std::vector<ShaderUniform>()));
        if (!flavor) continue;
        gl.iglUseProgram(flavor->GetGLId());
        UploadAutoUniforms(gl, *flavor, pass, job, mat);
        if (job.GetSkinWeightCount() && job.GetNodeTransformCount()) {
            // the node palette of the skinned flavor: three rows per node
            int loc = UniformLocation(gl, flavor->GetGLId(), jet::String("_u_node_transforms_"));
            if (loc >= 0) {
                unsigned n = job.GetNodeTransformCount();
                if (n > flavor->GetMaxSkinNodeCount()) n = flavor->GetMaxSkinNodeCount();
                std::vector<float> rows(n * 12);
                for (unsigned k = 0; k < n; k++) {
                    const math::mat4<float> &t = job.GetNodeTransform(k);
                    float *r = &rows[k * 12];
                    r[0] = t.m[0]; r[1] = t.m[4]; r[2] = t.m[8]; r[3] = t.m[12];
                    r[4] = t.m[1]; r[5] = t.m[5]; r[6] = t.m[9]; r[7] = t.m[13];
                    r[8] = t.m[2]; r[9] = t.m[6]; r[10] = t.m[10]; r[11] = t.m[14];
                }
                if (n) gl.iglUniform4fv(loc, (int)n * 3, &rows[0]);
            }
        }
        ApplyMaterial(gl, *flavor, mat, p, job);
        static_cast<GLES20Geometry*>(job.GetGeometry().get())->Draw(job, mat, p, *flavor);
    }
    return true;
}

}
