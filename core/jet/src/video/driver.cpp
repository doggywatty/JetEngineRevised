// core/jet/src/video/driver.cpp
#include "video/driver.h"
#include "scene/camera.h"
#include "video/display.h"
#include <cmath>

namespace jet::video {

Driver::Driver()
    : m_cameraParamsIdx(0), m_isLocked(false), m_frameIdx(0), m_isUnloaded(false),
      m_globalUniformCrtSetIdx(0), m_lastTagIdx(0), m_tagMask(0xffffffff),
      m_solidSortKey(SortKeyDef::Material), m_driverTime(0), m_driverWrapTime(0),
      m_driverFracTime(0), m_postEffectAASamples(0), m_useSeparateAlphaTextures(false),
      m_depthRangeNear(0), m_depthRangeFar(1.0f) {
    // Always keep at least one (identity) camera-params entry so GetCrtCameraParams() is safe
    // to call before any camera has pushed real view/projection matrices.
    m_cameraParams.push_back(CameraParams());
}
Driver::~Driver() {}

bool Driver::SetDisplay(Display*) { return true; }

void Driver::UnloadGPUResources() { m_isUnloaded = true; }
void Driver::ReloadGPUResources() { m_isUnloaded = false; }

// AddTime @0x6e9cec: the wrap time drops to its fractional part once it passes 30 s
void Driver::AddTime(int ms) {
    float dt = ms * 0.001f;
    m_driverTime += dt;
    m_driverWrapTime += dt;
    if (m_driverWrapTime > 30.0f) m_driverWrapTime -= floorf(m_driverWrapTime);
    m_driverFracTime += dt;
    m_driverFracTime -= floorf(m_driverFracTime);
}
void Driver::SetCameraParams(const CameraParams &p) { m_cameraParams[m_cameraParamsIdx] = p; }
float Driver::GetTime() { return m_driverTime; }
float Driver::GetFracTime() { return m_driverFracTime; }
float Driver::GetWrapTime() { return m_driverWrapTime; }

void Driver::SetPostEffectAASamples(unsigned n) { m_postEffectAASamples = n; }
unsigned Driver::GetPostEffectAASamples() { return m_postEffectAASamples; }

void Driver::AddShaderDefine(const jet::String &name, const jet::String &value) { m_shaderDefines[name] = value; }
void Driver::AddShaderDefine(const jet::String &name) { m_shaderDefines[name] = jet::String(); }
void Driver::RemoveShaderDefine(const jet::String &name) { m_shaderDefines.erase(name); }
const std::map<jet::String,jet::String> &Driver::GetShaderDefinesMap() { return m_shaderDefines; }

void Driver::GetDepthRange(float &n, float &f) { n = m_depthRangeNear; f = m_depthRangeFar; }
void Driver::SetDepthRange(float n, float f) { m_depthRangeNear = n; m_depthRangeFar = f; }

void Driver::SetSolidPassSortKey(safe_enum<SortKeyDef,SortKeyDef::type> k) { m_solidSortKey = k; }
safe_enum<Driver::SortKeyDef,Driver::SortKeyDef::type> Driver::GetSolidPassSortKey() { return m_solidSortKey; }

void Driver::SetSeparateAlphaTextures(bool v) { m_useSeparateAlphaTextures = v; }
bool Driver::GetSeparateAlphaTextures() { return m_useSeparateAlphaTextures; }

bool Driver::BeginRendering() {
    if (m_isLocked) return false;
    m_isLocked = true;
    m_frameIdx++;
    return true;
}
bool Driver::EndRendering() { m_isLocked = false; return true; }
bool Driver::IsLocked() { return m_isLocked; }
unsigned Driver::GetFrameIdx() { return m_frameIdx; }

int Driver::AddRenderTarget(const boost::shared_ptr<RenderTarget> &rt) {
    int idx = (int)m_renderTargets.size();
    m_renderTargets.push_back(rt);
    m_renderTargetsIdxMap[rt->GetName()] = idx;
    return idx;
}
int Driver::DeclareRenderTarget(const jet::String &name) {
    int existing = FindRenderTargetIdxByName(name);
    if (existing >= 0) return existing;
    return AddRenderTarget(boost::shared_ptr<RenderTarget>());
}
const boost::shared_ptr<RenderTarget> &Driver::GetRenderTarget(unsigned i) { return m_renderTargets[i]; }
jet::String Driver::GetRenderTargetName(unsigned i) { return m_renderTargets[i] ? m_renderTargets[i]->GetName() : jet::String(); }
int Driver::FindRenderTargetIdxByName(const jet::String &name) {
    if (name.Equals("default")) return k_defaultRenderTargetIdx;
    if (name.Equals("previous")) return k_previousRenderTargetIdx;
    boost::unordered_map<jet::String,unsigned>::iterator it = m_renderTargetsIdxMap.find(name);
    return it == m_renderTargetsIdxMap.end() ? -1 : (int)it->second;
}
const boost::shared_ptr<RenderTarget> &Driver::FindRenderTargetByName(const jet::String &name) {
    int idx = FindRenderTargetIdxByName(name);
    return idx < 0 || idx >= (int)m_renderTargets.size() ? m_emptyRTPtr : m_renderTargets[idx];
}
const boost::shared_ptr<RenderTarget> &Driver::GetCrtRenderTarget() { return m_crtRenderTarget; }
void Driver::SetCrtRenderTarget(const boost::shared_ptr<RenderTarget> &rt) { m_crtRenderTarget = rt; }

unsigned Driver::GetTag(const jet::String &name) {
    boost::unordered_map<jet::String,unsigned>::iterator it = m_tagNames.find(name);
    if (it != m_tagNames.end()) return it->second;
    unsigned idx = ++m_lastTagIdx;  // @0x6e9d80: the first named tag is 1, 0 is the default tag
    m_tagNames[name] = idx;
    return idx;
}
const jet::String &Driver::GetTagName(unsigned idx) {
    for (boost::unordered_map<jet::String,unsigned>::iterator it = m_tagNames.begin(); it != m_tagNames.end(); ++it)
        if (it->second == idx) return it->first;
    static jet::String empty;
    return empty;
}
unsigned Driver::GetTagCount() { return m_lastTagIdx; }
unsigned Driver::GetTagMask() { return m_tagMask; }
void Driver::SetTagMask(unsigned m) { m_tagMask = m; }
void Driver::ClearTagMask() { m_tagMask = 0; }

// SetCamera @0x6e95ac: the camera's position, axes and matrices become the current camera params
void Driver::SetCamera(const jet::scene::Camera *c) {
    if (!c) return;
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(*c);
    CameraParams p;
    p.view = cam.GetViewTransform();
    p.projection = cam.GetProjectionTransformOriented();
    p.position = cam.GetPosition();
    p.front = cam.GetFrontVector();
    p.right = cam.GetRightVector();
    p.up = cam.GetUpVector();
    SetCameraParams(p);
}

unsigned Driver::SetGlobalTexture(const jet::String &name, const boost::shared_ptr<Texture> &t) {
    int idx = FindGlobalTextureIdxByName(name);
    if (idx >= 0) { m_globalTextures[idx] = t; return idx; }
    idx = (int)m_globalTextures.size();
    m_globalTextures.push_back(t);
    m_nameToGlobalTextureIdx[name] = idx;
    return idx;
}
boost::shared_ptr<Texture> Driver::FindGlobalTextureByName(const jet::String &name) {
    int idx = FindGlobalTextureIdxByName(name);
    return idx < 0 ? boost::shared_ptr<Texture>() : m_globalTextures[idx];
}
int Driver::FindGlobalTextureIdxByName(const jet::String &name) {
    boost::unordered_map<jet::String,unsigned>::iterator it = m_nameToGlobalTextureIdx.find(name);
    return it == m_nameToGlobalTextureIdx.end() ? -1 : (int)it->second;
}
const boost::shared_ptr<Texture> &Driver::GetGlobalTexture(unsigned i) { return m_globalTextures[i]; }

unsigned Driver::SetGlobalUniform(const jet::String &name, const ShaderUniform &u) {
    int idx = FindGlobalUniformIdxByName(name);
    if (idx < 0) {
        idx = (int)m_globalUniforms.size();
        m_globalUniforms.push_back(std::vector<ShaderUniform>());
        m_nameToGlobalUniformIdx[name] = idx;
    }
    std::vector<ShaderUniform> &set = m_globalUniforms[idx];
    if (set.size() <= m_globalUniformCrtSetIdx) set.resize(m_globalUniformCrtSetIdx + 1);
    set[m_globalUniformCrtSetIdx] = u;
    return idx;
}
int Driver::FindGlobalUniformIdxByName(const jet::String &name) {
    boost::unordered_map<jet::String,unsigned>::iterator it = m_nameToGlobalUniformIdx.find(name);
    return it == m_nameToGlobalUniformIdx.end() ? -1 : (int)it->second;
}
const ShaderUniform &Driver::GetGlobalUniform(unsigned idx, unsigned set) const { return m_globalUniforms[idx][set]; }
ShaderUniform &Driver::GetGlobalUniform(unsigned idx, unsigned set) { return m_globalUniforms[idx][set]; }
const ShaderUniform *Driver::FindGlobalUniformByName(const jet::String &name) const {
    boost::unordered_map<jet::String,unsigned>::const_iterator it = m_nameToGlobalUniformIdx.find(name);
    return it == m_nameToGlobalUniformIdx.end() ? 0 : &m_globalUniforms[it->second][m_globalUniformCrtSetIdx];
}
ShaderUniform *Driver::FindGlobalUniformByName(const jet::String &name) {
    int idx = FindGlobalUniformIdxByName(name);
    return idx < 0 ? 0 : &m_globalUniforms[idx][m_globalUniformCrtSetIdx];
}

const Driver::CameraParams &Driver::GetCameraParams(unsigned i) { return m_cameraParams[i]; }
unsigned Driver::GetCrtCameraParamsIdx() { return m_cameraParamsIdx; }
const Driver::CameraParams &Driver::GetCrtCameraParams() { return m_cameraParams[m_cameraParamsIdx]; }

void Driver::GetColorBufferData(const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, unsigned char*) {}
void Driver::GetColorBufferData(unsigned char*) {}
void Driver::GetDepthBufferData(const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, float*) {}

namespace { Driver *g_currentDriver = 0; }
void Driver::SetCurrent(Driver *d) { g_currentDriver = d; }
Driver *Driver::GetCurrent() { return g_currentDriver; }

}
