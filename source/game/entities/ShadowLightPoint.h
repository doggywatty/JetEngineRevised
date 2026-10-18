// source/game/entities/ShadowLightPoint.h
#ifndef SOURCE_GAME_ENTITIES_SHADOWLIGHTPOINT_H
#define SOURCE_GAME_ENTITIES_SHADOWLIGHTPOINT_H

#include "jsingleton.h"
#include "GameEntity.h"

namespace jet::scene { class Model; }

// The "ShadowLightPoint" template (0x94 bytes in the 0.0.3 .so): the point the shadow map's light looks at.
// Each frame it points the game's shadow camera from the light (this entity's position) at the model the
// script named with SetShadowLightPoint (ApplyRefPoint @0x521b48)
class ShadowLightPoint : public GameEntity, public Singleton<ShadowLightPoint> {  // sizeof 0x94
public:
    explicit ShadowLightPoint(const clara::Template*);
    virtual ~ShadowLightPoint();
    virtual void Init();
    virtual void SetEnabled(bool);
    virtual void SetVisible(bool);
    virtual void Render();
    virtual void NotifyTransformChanged(bool);
    // points the game's shadow camera from the light at the model's reference node
    static void ApplyRefPoint(const math::vec3<float>&);
    // SetShadowLightPoint's model (@0x4261a0 stores a Deco3d's)
    static void SetModelRefPoint(jet::scene::Model *m) { m_modelRefPoint = m; }
    static jet::String k_tmplName;
private:
    static jet::scene::Model *m_modelRefPoint;
    static bool m_canUseFirstSkeletalChild;
};

#endif
