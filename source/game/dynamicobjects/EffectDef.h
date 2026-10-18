// source/game/dynamicobjects/EffectDef.h
#ifndef SOURCE_GAME_DYNAMICOBJECTS_EFFECTDEF_H
#define SOURCE_GAME_DYNAMICOBJECTS_EFFECTDEF_H

#include <vector>
#include "game/entities/GameEntity.h"

class LightDef;
class TrailDefEntity;

// "EffectDef" template (0x100 bytes in the 0.0.3 .so; ctor @0x59a8d0, Init @0x59aa94): what an effect is made of -- a model with its animation and
// materials, a particle system, a light, a trail, a sound, a screen fade and shake, and the animations of the model that flies in and bounces on
// the screen. The EffectMgr plays it.
class EffectDef : public GameEntity {
public:
    struct BounceSide { enum type { Left = 0, Right = 1, Random = 2 }; };

    explicit EffectDef(const clara::Template*);
    virtual ~EffectDef();
    virtual void Init();

    bool IsLooped() const { return m_isLooped; }
    bool RotateModel() const { return m_rotateModel; }
    bool IsImpostor() const { return m_isImpostor; }
    const jet::String &GetModelName() const { return m_modelName; }                  // @0x59b278
    const jet::String &GetAnimName() const { return m_animName; }                    // @0x59b280
    const MaterialDefs &GetMaterialDef() const { return m_materials; }               // @0x59b288
    const jet::String &GetShadowModelName() const { return m_shadowModelName; }      // @0x59b290
    const MaterialDefs &GetShadowMaterialDef() const { return m_shadowMaterials; }   // @0x59b298
    LightDef *GetLightDef() const { return m_lightDef; }
    int GetLightTime() const { return m_lightTime; }
    const jet::String &GetSoundLabel() const { return m_soundLabel; }
    const jet::String &GetParticleSystem() const { return m_particles; }
    bool GetUseFlashScreen() const { return m_useFade; }
    TrailDefEntity *GetTrailDef() const { return m_trailDef; }
    bool GetUseBounceScreenAnim() const { return m_useBounceScreenAnim; }
    int GetBounceSide() const { return m_bounceSide; }
    const std::vector<jet::String> &GetFlyingScreenAnim() const { return m_flyingScreenAnim; }
    const std::vector<jet::String> &GetBounceScreenAnim() const { return m_bounceScreenAnim; }
    // @0x59b214: the screen fades through the effect's technique
    void FlashScreen() const;
    // @0x59b2a0: the current camera shakes for ShakeCameraTime
    void ShakeCamera() const;

    static jet::String k_tmplName;
private:
    bool m_isLooped;                         // 0x8C: UseLoop
    bool m_rotateModel;                      // 0x8D
    bool m_isImpostor;                       // 0x8E
    jet::String m_modelName;                 // 0x90
    jet::String m_animName;                  // 0x94
    MaterialDefs m_materials;                // 0x98
    jet::String m_shadowModelName;           // 0xA4
    MaterialDefs m_shadowMaterials;          // 0xA8
    LightDef *m_lightDef;                    // 0xB4
    int m_lightTime;                         // 0xB8
    jet::String m_soundLabel;                // 0xBC: ImpactSoundEvent
    jet::String m_particles;                 // 0xC0: Emmiter (EmmiterLow on the devices with low particles)
    bool m_useFade;                          // 0xC4: UseFade
    jet::String m_fadeTechnique;             // 0xC8: Shader
    int m_fadeInTime, m_fadePeakTime, m_fadeOutTime;  // 0xCC, 0xD0, 0xD4
    float m_shakeCameraTime;                 // 0xD8
    bool m_useBounceScreenAnim;              // 0xDC
    std::vector<jet::String> m_flyingScreenAnim;  // 0xE0
    std::vector<jet::String> m_bounceScreenAnim;  // 0xEC
    int m_bounceSide;                        // 0xF8: bounceScreenPosition
    TrailDefEntity *m_trailDef;              // 0xFC: TrailData
};

#endif
