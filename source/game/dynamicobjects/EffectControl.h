// source/game/dynamicobjects/EffectControl.h
#ifndef SOURCE_GAME_DYNAMICOBJECTS_EFFECTCONTROL_H
#define SOURCE_GAME_DYNAMICOBJECTS_EFFECTCONTROL_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "particlesystem.h"

namespace jet::scene { class Model; class Node; class Dummy; }
class EffectDef;
class LightDef;
class CameraController;
class EffectControl;
class Trail;

// Who is told when an effect stops by itself or is released (vtable @0xce5310): the state machine keeps the effects its states started
class EffectControlListener {
public:
    virtual ~EffectControlListener() {}
    virtual void ECL_OnStop(EffectControl*) = 0;     // EffectMgr::Stop @0x59ce54
    virtual void ECL_OnRelease(EffectControl*) = 0;  // EffectMgr::Release @0x59c120
};

// One of the 200 slots of the EffectMgr (0x70 bytes): what an EffectDef is playing. The manager hands out its slots, they are never allocated.
class EffectControl {
public:
    struct BounceScreenState { enum type { None = 0, Flying = 1, Bouncing = 2, Done = 3 }; };

    EffectControl()
        : layerId(0), model(0), def(0), node(0), hasParticles(false), active(false), autoRemove(false), visible(false), lifeTime(0), light(0),
          lightTime(-1), trail(0), position(0, 0, 0), tag(0), bounceState(0), bounceTime(0), bounceCamera(0), bounceX(0), bounceY(0),
          soundHandle(-1), listener(0) {}

    int layerId;                                        // 0x00: the layer it is drawn and updated in
    jet::scene::Model *model;                           // 0x04: from the ModelPool
    const EffectDef *def;                               // 0x08
    const jet::scene::Node *node;                       // 0x0C: what it follows
    bool hasParticles;                                  // 0x10
    boost::shared_ptr<ps::ParticleSystem> particles;    // 0x14
    bool active;                                        // 0x1C: the slot is in use
    bool autoRemove;                                    // 0x1D: released by the manager when it ends
    bool visible;                                       // 0x1E
    int lifeTime;                                       // 0x20: ms left, -1 for ever
    LightDef *light;                                    // 0x24
    int lightTime;                                      // 0x28: ms of light left, -1 for ever
    Trail *trail;                                       // 0x2C: the trail of the effect (the TrailMgr is not ported: never made)
    math::vec3<float> position;                         // 0x30
    long long tag;                                      // 0x40: who started it (the state machine's key)
    int bounceState;                                    // 0x48: BounceScreenState
    float bounceTime;                                   // 0x4C
    CameraController *bounceCamera;                     // 0x50
    float bounceX, bounceY;                             // 0x54, 0x58: where on the screen it ends up
    int soundHandle;                                    // 0x5C: the sound it plays (the SoundMgr is not ported: never made)
    std::vector<jet::scene::Dummy*> animatedDummies;    // 0x60: "Animated_Dummy<n>" of the model
    EffectControlListener *listener;                    // 0x6C
};

#endif
