// source/game/entities/Deco3d.h
#ifndef SOURCE_GAME_ENTITIES_DECO3D_H
#define SOURCE_GAME_ENTITIES_DECO3D_H

#include "GameEntity.h"
#include "core/delayer.h"
#include "game/GameLevel.h"
#include "game/utils/GameUtils.h"

namespace jet::scene { class Model; class Node; }
namespace jet::anim { class Animation; }

// A placed model ("Deco3d" template, 0x1B4 bytes in the 0.0.3 .so): loads its Model, animates it, adds it to
// the level's scene manager each frame, optionally with a shadow model, an alpha fade and random animations.
// Not ported yet: the light receiver and the trail (LightReceiver, TrailMgr).
class Deco3d : public GameEntity {
public:
    // how a new animation blends from the one playing: not at all, only when that one is still going, always
    struct AnimInterpolation { enum Enum { None = 0, IfPlaying = 1, Always = 2 }; };
    struct ShadowAnimatedNodes { jet::scene::Node *shadowNode, *modelNode; };

    explicit Deco3d(const clara::Template*);
    Deco3d(Deco3d*, unsigned ctorFlags = 0);
    virtual ~Deco3d();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void SetEnabled(bool);
    virtual void SetVisible(bool);
    virtual bool GetLinkPointTransform(const jet::String&, math::vec3<float>&, math::quat<float>&) const;
    virtual void NotifyAnimationChanged();
    virtual void NotifyTransformChanged(bool);
    virtual float GetFocusRadius() const;

    jet::scene::Model *GetModel() const { return m_model; }
    jet::anim::Animation *GetAnimation() const;
    bool CanLoadModel() const;
    const jet::String &GetModelName() const { return m_modelName; }
    const jet::String &GetAnimationName() const { return m_animationName; }
    void SetAnimationName(const jet::String &n) { m_animationName = n; }

    void SetAnimation(const jet::String &name, bool loop, AnimInterpolation::Enum = AnimInterpolation::IfPlaying);
    void SetTexAnimation(const jet::String &name, bool loop);
    void SetAnimationSpeed(float speed);
    void SetAnimationTime(unsigned ms);
    void SetAnimationTimeToReach(int ms) { m_animationTimeToReach = ms; }
    int GetAnimTime() const;
    int GetAnimOldTime() const;
    void SetForceAnimation(bool b) { m_forceAnimation = b; }
    void PlayRandomAnimations(const std::vector<jet::String>&);
    void StopRandomAnimations() { m_randomAnimations.clear(); }

    void SetRotationToReach(const math::quat<float>&, unsigned ms);
    bool IsRotating() const { return m_rotationToReachDelayer.IsUpdating(); }
    void SetScaleToReach(const math::vec3<float>&, unsigned ms);
    bool IsScaling() const { return m_scaleToReachDelayer.IsUpdating(); }

    void SetMaterial(const jet::String&, MaterialDef*);
    void SetAllMaterials(MaterialDef*);
    void TriggerAlphaFade(float alpha, int ms);
    float GetAlpha() const { return m_alphaDelayer.GetCurrentValue(); }
    void SetModelNodesVisible(const jet::String &prefix, bool);
    void SetShadowEnabled(bool b) { m_shadowEnabled = b; }
    void SetSceneMgr(GameLevel::SceneMgrId id) { m_sceneMgrId = id; }
    void Blink(int ms) { m_blinkingTimer = ms; }
    void SetMovieLinkedObject(void *o) { m_movieLinkedObject = o; }
    const math::aabb<float> &GetCurrentAabb() const { return m_currentAabb; }

    struct TModelData {
        jet::String model, animation;
        bool useMultiMaterials, loop;
        MaterialDefs materials;
    };
    void FillModelData(TModelData&);
    void ApplyModelData(const TModelData&);

    static jet::String k_tmplName;
    static const jet::String k_model;
    static const jet::String k_lowModel;
    static const jet::String k_animation;
    static const jet::String k_texAnimation;
    static const jet::String k_useTexAnimationLoop;
    static const jet::String k_animSpeed;
    static const jet::String k_useLoop;
    static const jet::String k_useMultiMaterials;
    static const jet::String k_lowUseMultiMaterials;
    static const jet::String k_ChooseRandAnimOnAnimEnd;
    static const jet::String k_useMovementDummy;
    static const jet::String k_materials;
    static const jet::String k_lowMaterials;
    static const jet::String k_trailData;
    static const jet::String k_alphaScale;
    static const jet::String k_shadowModel;
    static const jet::String k_shadowMaterial;
    static const jet::String k_updateAnimOnUpdate;
    static const jet::String k_generationGroup;
    static const jet::String k_optional;

protected:
    void CheckAlphaFadeability();
    void InitEntity();
    void InitShadowNodes(jet::scene::Model*, jet::scene::Model*);
    void UpdateShadowNodes();
    void ApplyAlpha();

    jet::scene::Model *m_shadowModel;                            // 0x8c
    bool m_shadowEnabled;                                        // 0x90
    MaterialDefs m_shadowMaterialDefs;                           // 0x94
    std::vector<ShadowAnimatedNodes> m_shadowAnimatedNodes;      // 0xa0
    jet::String m_modelName, m_animationName, m_texAnimationName;  // 0xac
    jet::scene::Model *m_model;                                  // 0xb8
    jet::core::Delayer<math::quat<float>, jet::core::OcticEaseOut> m_rotationToReachDelayer;  // 0xbc
    jet::core::Delayer<math::vec3<float>, jet::core::OcticEaseOut> m_scaleToReachDelayer;     // 0xf8
    int m_animationTimeToReach;                                  // 0x128
    bool m_useMovementDummy;                                     // 0x12c
    bool m_useMultiMaterial, m_lowUseMultiMaterial;              // 0x12d
    MaterialDefs m_materialDefs;                                 // 0x130
    unsigned m_currentRandomAnimIdx;                             // 0x13c
    std::vector<jet::String> m_randomAnimations;                 // 0x140
    jet::core::Delayer<float, jet::core::SmoothStep> m_alphaDelayer;  // 0x14c
    bool m_triggeredAlphaFade;                                   // 0x164
    std::vector<ShaderUniformRef> m_alphaScaleShaderUniforms;    // 0x168
    void *m_movieLinkedObject;                                   // 0x174
    GameLevel::SceneMgrId m_sceneMgrId;                          // 0x188
    bool m_updateAabb;                                           // 0x18c
    math::aabb<float> m_currentAabb;                             // 0x190
    int m_blinkingTimer;                                         // 0x1a8
    bool m_forceAnimation;                                       // 0x1ac
    unsigned m_ctorFlags;                                        // 0x1b0
};

#endif
