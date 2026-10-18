// core/jet/include/video/gles20/gles20driver.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20DRIVER_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20DRIVER_H

#include <set>
#include <vector>
#include <map>
#include "video/driver.h"
#include "video/gles20/gles20interface.h"
#include "video/gles20/gles20geometry.h"
#include "video/gles20/gles20texture.h"
#include "video/gles20/gles20shaderprogram.h"
#include "video/renderjob.h"
#include "video/material.h"

namespace jet::video {

// A real, GL-driving implementation of Driver, deliberately simplified from the original's
// architecture in ways that don't change what ends up on screen:
//  - DrawPrimitives applies render state/shader/uniforms/textures and draws immediately, instead
//    of queuing a RenderNode into solid/alphaTest/transparent lists for later sorted, batched
//    flushing (GLES20Driver::FlushNodes/ShaderSorter/Back2FrontSorter/CreateBatchedRenderNode in
//    the 0.0.3 .so). Batching/sorting are throughput optimizations; per-draw-call correctness is
//    unaffected by doing them one at a time in submission order.
//  - No render-node/render-task object pooling (FSBAllocator<RenderNode>, m_renderTaskPool):
//    nothing here is on a separate render thread yet, so there is nothing to hand work off to.
//  - Lighting is uploaded as a flat array of active lights' uniforms rather than split into
//    directional/point light-processing passes (m_dlights/m_plights/ProcessLight); shaders that
//    want per-light-type behavior read GetLightCount()/GetLight(i) and branch themselves.
class GLES20Driver : public Driver {
public:
    GLES20Driver();
    virtual ~GLES20Driver();
    virtual bool Init(Display*);
    virtual void Shutdown(Display*);
    virtual Display *GetDisplay();
    virtual jet::String GetArchitectureString();
    virtual void SetMaxLightCount(unsigned);
    virtual unsigned GetMaxLightCount();
    virtual unsigned GetMaxTextureUnits();
    virtual void SetBatchVertexLimit(unsigned);
    // Overridden to actually bind/clear/set-viewport for the new target (via
    // GLES20RenderTargetWrapper::Activate) -- Driver's base setter only stores the pointer.
    virtual void SetCrtRenderTarget(const boost::shared_ptr<RenderTarget>&);
    virtual bool BeginRendering();
    virtual bool EndRendering();
    virtual void FinishAllRendering();
    virtual void BeginPostEffect(const std::vector<PostEffect*>&, safe_enum<PostFXAADef,PostFXAADef::type>);
    virtual void BeginPostEffect(PostEffect*, safe_enum<PostFXAADef,PostFXAADef::type>);
    virtual void EndPostEffect(const math::mat4<float>&);
    virtual bool AddLight(const LightData&);
    virtual void ResetLights();
    virtual void SetFog(bool);
    virtual bool GetFog();
    virtual void SetFogParams(const Color&, float, float);
    virtual void GetFogParams(Color&, float&, float&);
    virtual bool DrawPrimitives(const RenderJob&);
    virtual void Flush();
    virtual void SyncAll();
    virtual bool IsFeatureSupported(safe_enum<Driver::FeatureDef,Driver::FeatureDef::type>);
protected:
    virtual bool LinkUniform(ShaderUniform&, const jet::String&);
    // render target links, beside LinkUniform's engine value ids (< 0xb0) and globals (0xb0+)
    int UniformLocation(gles::Interface&, unsigned program, const jet::String&);
    std::map<std::pair<unsigned, jet::String>, int> m_uniformLocations;
    static const unsigned k_linkRenderTargetRatio = 0x1000, k_linkRenderTargetSize = 0x2000;
    virtual bool LinkSampler(ShaderSampler&, const jet::String&);
    virtual boost::shared_ptr<RenderTarget> CreateRenderTarget();
    virtual ShaderProgram *CreateShaderProgram(RenderTechnique*);
    virtual boost::shared_ptr<Texture> CreateTexture();
    virtual boost::shared_ptr<Geometry> CreateGeometry();
    void InitFeatures();
    void ApplyRenderState(const RenderState&);
    void ApplyMaterial(gles::Interface&, GLES20ShaderProgramFlavor&, const Material&, unsigned pass, const RenderJob&);
    void UploadAutoUniforms(gles::Interface&, GLES20ShaderProgramFlavor&, RenderPass&, const RenderJob&, const Material&);
    // Post effects: targets "effect<i>" of the display, the chain being drawn and its input
    boost::shared_ptr<RenderTarget> GetPostEffectRenderTarget(unsigned);
    boost::shared_ptr<RenderTarget> ResolveRenderTarget(unsigned idx);
    std::vector<PostEffect*> m_postEffects;
    boost::shared_ptr<RenderTarget> m_postEffectFinalTarget;
    boost::shared_ptr<RenderTarget> m_previousRenderTarget;
    boost::shared_ptr<Geometry> m_postEffectQuad;
    unsigned m_whiteAlphaTexture = 0;  // bound for alpha samplers of layers without an alpha texture
private:
    Display *m_display;
    bool m_betweenBeginEndRendering;
    std::set<int> m_supportedFeatures;
    bool m_isFogEnabled;
    Color m_fogColor;
    float m_fogStart, m_fogEnd;
    unsigned m_maxTextureUnits;
    unsigned m_maxLightCount;
    std::vector<LightData> m_lights;
    unsigned m_batchingVertexLimit;
    unsigned m_flushCount;
    RenderState m_appliedRenderState;
    bool m_hasAppliedRenderState;
};
}

#endif
