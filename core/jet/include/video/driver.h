// core/jet/include/video/driver.h
#ifndef CORE_JET_INCLUDE_VIDEO_DRIVER_H
#define CORE_JET_INCLUDE_VIDEO_DRIVER_H

#include <vector>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include "jenum.h"
#include "smath.h"
#include "core/pstring.h"
#include "thread/mutex.h"
#include "video/rendertarget.h"
#include "video/texture.h"
#include "video/shaderuniform.h"
#include "video/shadersampler.h"
#include "video/lightdata.h"
#include "video/color.h"

namespace jet::scene { class Camera; }
namespace jet::thread { class Task; struct TypeDef; }

namespace jet::video {
class Display;
class RenderJob;
class RenderTechnique;
class RenderPass;
class ShaderProgram;
class Geometry;
class PostEffect;

struct PostFXAADef { enum type { None, FXAA }; };

// Debug-overlay/GPU-resource-reload details from the original (dbg::DebugContext fields, the
// EndFrame/UnloadResource callback lists) are trimmed here since jet::dbg isn't reimplemented;
// everything that affects actual rendering (render targets, tags, globals, camera stack, fog/
// light bookkeeping, draw dispatch) is real.
class Driver : public boost::noncopyable {
public:
    struct SortKeyDef { enum type { Material, DrawLayer, Distance }; };
    struct FeatureDef { enum type { NPOTTextures, DepthTexture, MSAA, VertexArrayObject, MapBuffer, Anisotropy }; };

    // Current-device pointer: needed by subsystems (Painter) that draw without owning a Driver
    // reference themselves. Not part of the PDB-declared Driver surface -- this is real plumbing
    // this rebuild needs since the System/Application layer that would otherwise own/pass the
    // driver isn't implemented yet.
    static void SetCurrent(Driver*);
    static Driver *GetCurrent();

    // what SetCamera @0x6e95ac records of the camera: position, front/right/up and the matrices
    struct CameraParams {
        math::mat4<float> view;
        math::mat4<float> projection;
        math::vec3<float> position;
        math::vec3<float> front, right, up;
        CameraParams() : position(0, 0, 0), front(0, 1, 0), right(1, 0, 0), up(0, 0, 1) {}
    };
    void SetCameraParams(const CameraParams&);

    Driver();
    virtual ~Driver();
    virtual bool Init(Display*) = 0;
    virtual void Shutdown(Display*) = 0;
    virtual bool SetDisplay(Display*);
    virtual Display *GetDisplay() = 0;
    // FindRenderTargetIdxByName's reserved names (@0x6e932c): "default" and "previous"
    static const unsigned k_defaultRenderTargetIdx = 0x2fb, k_previousRenderTargetIdx = 0x2fc;
    virtual void UnloadGPUResources();
    virtual void ReloadGPUResources();
    void AddTime(int ms);
    float GetTime();
    float GetFracTime();
    float GetWrapTime();
    void SetPostEffectAASamples(unsigned);
    unsigned GetPostEffectAASamples();
    virtual void SetBatchVertexLimit(unsigned) = 0;
    virtual jet::String GetArchitectureString() = 0;
    void AddShaderDefine(const jet::String&, const jet::String&);
    void AddShaderDefine(const jet::String&);
    void RemoveShaderDefine(const jet::String&);
    void GetDepthRange(float&, float&);
    void SetDepthRange(float, float);
    void SetSolidPassSortKey(safe_enum<SortKeyDef,SortKeyDef::type>);
    safe_enum<SortKeyDef,SortKeyDef::type> GetSolidPassSortKey();
    void SetSeparateAlphaTextures(bool);
    bool GetSeparateAlphaTextures();
    virtual bool BeginRendering();
    virtual bool EndRendering();
    virtual void FinishAllRendering() = 0;
    virtual bool IsFeatureSupported(safe_enum<FeatureDef,FeatureDef::type>) = 0;
    bool IsLocked();
    virtual void BeginPostEffect(const std::vector<PostEffect*>&, safe_enum<PostFXAADef,PostFXAADef::type>) = 0;
    virtual void BeginPostEffect(PostEffect*, safe_enum<PostFXAADef,PostFXAADef::type>) = 0;
    virtual void EndPostEffect(const math::mat4<float>&) = 0;
    int AddRenderTarget(const boost::shared_ptr<RenderTarget>&);
    int DeclareRenderTarget(const jet::String&);
    const boost::shared_ptr<RenderTarget> &GetRenderTarget(unsigned);
    jet::String GetRenderTargetName(unsigned);
    const boost::shared_ptr<RenderTarget> &FindRenderTargetByName(const jet::String&);
    int FindRenderTargetIdxByName(const jet::String&);
    const boost::shared_ptr<RenderTarget> &GetCrtRenderTarget();
    virtual void SetCrtRenderTarget(const boost::shared_ptr<RenderTarget>&);
    unsigned GetTag(const jet::String&);
    const jet::String &GetTagName(unsigned);
    unsigned GetTagCount();
    unsigned GetTagMask();
    void SetTagMask(unsigned);
    void ClearTagMask();
    virtual void SetCamera(const jet::scene::Camera*);
    virtual void SetFog(bool) = 0;
    virtual bool GetFog() = 0;
    virtual void SetFogParams(const Color&, float, float) = 0;
    virtual void GetFogParams(Color&, float&, float&) = 0;
    virtual void SetMaxLightCount(unsigned) = 0;
    virtual unsigned GetMaxLightCount() = 0;
    virtual bool AddLight(const LightData&) = 0;
    virtual void ResetLights() = 0;
    virtual unsigned GetMaxTextureUnits() = 0;
    virtual bool DrawPrimitives(const RenderJob&) = 0;
    virtual void Flush() = 0;
    virtual void SyncAll() = 0;
    unsigned GetFrameIdx();
    virtual void GetColorBufferData(const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, unsigned char*);
    virtual void GetColorBufferData(unsigned char*);
    virtual void GetDepthBufferData(const math::vec2<unsigned int>&, const math::vec2<unsigned int>&, float*);
    unsigned SetGlobalTexture(const jet::String&, const boost::shared_ptr<Texture>&);
    boost::shared_ptr<Texture> FindGlobalTextureByName(const jet::String&);
    unsigned SetGlobalUniform(const jet::String&, const ShaderUniform&);
    const ShaderUniform *FindGlobalUniformByName(const jet::String&) const;
    ShaderUniform *FindGlobalUniformByName(const jet::String&);
    const std::map<jet::String,jet::String> &GetShaderDefinesMap();
protected:
    friend class ShaderUniform;  // ParseValue links through the current driver
    friend class ShaderSampler;
    virtual bool LinkUniform(ShaderUniform&, const jet::String&) = 0;
    virtual bool LinkSampler(ShaderSampler&, const jet::String&) = 0;
    virtual ShaderProgram *CreateShaderProgram(RenderTechnique*) = 0;
    virtual boost::shared_ptr<RenderTarget> CreateRenderTarget() = 0;
    virtual boost::shared_ptr<Texture> CreateTexture() = 0;
    virtual boost::shared_ptr<Geometry> CreateGeometry() = 0;
    int FindGlobalTextureIdxByName(const jet::String&);
    const boost::shared_ptr<Texture> &GetGlobalTexture(unsigned);
    int FindGlobalUniformIdxByName(const jet::String&);
    const ShaderUniform &GetGlobalUniform(unsigned, unsigned) const;
    ShaderUniform &GetGlobalUniform(unsigned, unsigned);
    const CameraParams &GetCameraParams(unsigned);
    unsigned GetCrtCameraParamsIdx();
    const CameraParams &GetCrtCameraParams();
    static const int DEFAULT_RENDER_TARGET_IDX = -1;
    static const int PREVIOUS_RENDER_TARGET_IDX = -2;
    std::map<jet::String,jet::String> m_shaderDefines;
    std::vector<CameraParams> m_cameraParams;
    unsigned m_cameraParamsIdx;
    jet::thread::Mutex m_cameraParamsLock;
    bool m_isLocked;
    unsigned m_frameIdx;
    bool m_isUnloaded;
    boost::shared_ptr<RenderTarget> m_emptyRTPtr;
    boost::shared_ptr<RenderTarget> m_crtRenderTarget;
    std::vector<boost::shared_ptr<RenderTarget> > m_renderTargets;
    boost::unordered_map<jet::String,unsigned> m_renderTargetsIdxMap;
    boost::unordered_map<jet::String,unsigned> m_nameToGlobalTextureIdx;
    std::vector<boost::shared_ptr<Texture> > m_globalTextures;
    jet::thread::Mutex m_globalTextureMutex;
    boost::unordered_map<jet::String,unsigned> m_nameToGlobalUniformIdx;
    std::vector<std::vector<ShaderUniform> > m_globalUniforms;
    unsigned m_globalUniformCrtSetIdx;
    boost::unordered_map<jet::String,unsigned> m_tagNames;
    unsigned m_lastTagIdx;
    unsigned m_tagMask;
    safe_enum<SortKeyDef,SortKeyDef::type> m_solidSortKey;
    float m_driverTime;
    float m_driverWrapTime;
    float m_driverFracTime;
    unsigned m_postEffectAASamples;
    bool m_useSeparateAlphaTextures;
    float m_depthRangeNear;
    float m_depthRangeFar;
};
}

#endif
