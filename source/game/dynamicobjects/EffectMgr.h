// source/game/dynamicobjects/EffectMgr.h
#ifndef SOURCE_GAME_DYNAMICOBJECTS_EFFECTMGR_H
#define SOURCE_GAME_DYNAMICOBJECTS_EFFECTMGR_H

#include <vector>
#include "jsingleton.h"
#include "smath.h"
#include "EffectControl.h"

namespace jet::scene { class Model; class Node; }
class EffectDef;

// The effects (EffectDefs: models, particles, lights, screen fades and shakes, ...) played on models and in the world (ctor @0x59b314, 0x57a8
// bytes: 200 EffectControl slots at 0x8, the last one added at 0x5788 and the effects that bounce on the screen at 0x578c). The level and every
// menu scene that shows effects have a layer id of their own (GetNextLayerId; 0 is the level's): each updates and renders its layer.
// Not ported: the sound of the effects (SoundMgr is not in the tree) and their trails (TrailMgr).
class EffectMgr : public Singleton<EffectMgr> {
public:
    static const int k_maxEffects = 200;

    EffectMgr();
    virtual ~EffectMgr();

    // @0x59b544: at the origin, for ever, and the screen flashes when the definition says so; the control, or null when all slots are busy
    EffectControl *Add(int layerId, const EffectDef *def);
    // @0x59b5cc: the effect starts in a free slot (the control is then LastAdded); autoRemove: the manager releases it when it has ended;
    // node: what it follows; lifeTime: ms before it stops, -1 for ever; flashScreen: the screen fades when the definition has a fade
    bool Add(int layerId, const EffectDef *def, const math::vec3<float> &position, const math::quat<float> &rotation, const math::vec3<float> &scale, bool autoRemove,
             const jet::scene::Node *node, int lifeTime, bool flashScreen, EffectControlListener *listener);
    // @0x59bff4: the slot is free again
    void Release(EffectControl*);
    // @0x59cd48: the effect ends (its particles stop emitting) and is released by the manager when it is done
    void Stop(EffectControl*);
    // @0x59c240, @0x59c13c: the effects of a layer
    void Update(int layerId, int dt);
    void Render(int layerId);
    // @0x59c1e8
    bool Particles_HasParticlesFor(int layerId) const;
    // @0x59c86c, @0x59ca18, @0x59ca70, @0x59cc1c, @0x59cc3c
    void SetPosition(EffectControl*, const math::vec3<float>&);
    void SetRotation(EffectControl*, const math::quat<float>&);
    void SetTransform(EffectControl*, const math::vec3<float>&, const math::quat<float>&);
    void SetLinearVelocity(EffectControl*, const math::vec3<float>&);
    void SetVisible(EffectControl*, bool);
    // @0x59ce84: every effect moves with the world
    void Translate(const math::vec3<float>&);
    // @0x59ceec: what the effects followed on the old model they follow on the new one
    void OnModelChanged(jet::scene::Model *oldModel, jet::scene::Model *newModel);
    // @0x59cf6c: the effects that release themselves are released (or stopped)
    void CleanUpAutoRemoveEffects(bool release);
    // @0x59cfcc
    void ReleaseAutoRemoveEffectsAttachedToModel(jet::scene::Model*);
    // @0x59d060
    void OnEffectDefDestroyed(EffectDef*);
    EffectControl *GetLastAdded() const { return m_lastAdded; }
    // @0x59d038
    static int GetNextLayerId() { return ++s_lastUsedLayerId; }
    static const float k_bounceMoveToScreenTime;  // @0xc8bba4
private:
    // the effects that fly in from the side of the screen and bounce on it (EffectDef::UseBounceScreenAnim)
    void InitBounceScreenAnim(EffectControl*);                       // @0x59bac0
    void SetBounceScreenState(EffectControl*, int state);            // @0x59bbc8
    void UpdateBounceScreenAnim(EffectControl*, int dt);             // @0x59bd6c
    void UpdateBounceScreenPosition(EffectControl*);                 // @0x59be4c

    EffectControl m_controls[k_maxEffects];                          // 0x8
    EffectControl *m_lastAdded;                                      // 0x5788
    std::vector<EffectControl*> m_bounceScreenEffects;               // 0x578c
    static int s_lastUsedLayerId;
};

#endif
