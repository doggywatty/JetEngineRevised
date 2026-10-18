// source/game/entities/Actor.h
#ifndef SOURCE_GAME_ENTITIES_ACTOR_H
#define SOURCE_GAME_ENTITIES_ACTOR_H

#include "Object.h"
#include "JumpDefinition.h"
#include "param.h"
#include "game/actors/statemachine/StateMachine.h"

class Collision3d;

// An object driven by a state machine (its "StateMachine" param: set | state) with a gravity. 0x1CC bytes in the .so:
// Object, StateMachine at 0xC8, then the jump and gravity data. The gravity pulls it down the scene's up vector (or along the
// jump it follows), and what it falls on is found by casting a ray at the collisions of the physics world.
class Actor : public Object, public StateMachine {
public:
    explicit Actor(const clara::Template*);
    virtual ~Actor();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void InitStateMachine();
    virtual unsigned SM_GetId() { return GetId(); }  // @0x518cf8
    // @0x518d10: every actor is of the "Actor" base template, whatever its template is
    virtual const jet::String &GetBaseTemplateName() const { return k_tmplName; }
    virtual void SetCurrentJumpDefinition(JumpDefinition *j) { m_jump = j; }  // @0x518d08
    // @0x59f484: starts the jump at the place: gravity off, the control reset
    virtual void StartJumpDefinition(JumpDefinition*, const math::vec3<float> &pos, const math::quat<float> &rot);
    // the collision it last found under it (it is not out of the scene while it is)
    Collision3d *GetGroundCollision() const { return m_groundCollision; }
    static jet::String k_tmplName;
protected:
    // @0x59ec94, 0x55b5ac: the gravity along a direction (the scene's up vector when none), scaled
    void ResetGravity(math::vec3<float> &dir, float scale);
    void ResetGravity(float scale);
    // @0x59ed90: a jump of the given time (ms) and height under a gravity scale: the gravity that makes it, from the place it is at
    void ResetGravity(float timeMs, float height, float scale, bool timed);
    // @0x59eed0: the position goes on by the gravity (or the jump); when the ground is asked for and found, it is on it. True when it was put on the ground.
    bool UpdateGravity(math::vec3<float> &pos, bool projectOnGround);
    // @0x55b624: the ground below the point (a ray from 1 above to far below)
    bool ProjectOnGround(math::vec3<float> &pos);
    // @0x55b65c: a ray from the point along the reverse of the direction to the point along it
    bool ProjectOnGround(math::vec3<float> &pos, const math::vec3<float> &dir, float distance);
    // @0x55b708: a ray from above the point (up as the rotation says) by the first distance to below it by the second
    bool ProjectOnGround(math::vec3<float> &pos, math::quat<float> &rot, float below, float above);
    // @0x59f36c: the ray from the first point to the second: the first ground collision it meets is the place
    bool ProjectOnGround(const math::vec3<float> &from, const math::vec3<float> &to, math::vec3<float> &out);
    JumpDefinitionControl m_jumpCtrl;        // 0x158
    JumpDefinition *m_jump;                  // 0x17C
    math::vec3<float> m_jumpOrigin;          // 0x180: where the time-bound jump starts along the up axes
    math::vec3<float> m_jumpUp;              // 0x18C: how far up it goes
    float m_gravitySpeed[2];                 // 0x198: the time of a time-bound jump and how far it is
    math::vec3<float> m_gravityDir;          // 0x1A0: the speed the gravity has given
    math::vec3<float> m_gravity;             // 0x1AC
    Collision3d *m_groundCollision;          // 0x1B8
    clara::Param::StateValue m_stateMachine; // the data's set | state
    clara::Param::StateValue m_initialState; // what the state machine was started with
    static const jet::String k_stateMachine;
    static const float k_defaultGravity;
    static const float k_maxGravitySpeed;
};

#endif
