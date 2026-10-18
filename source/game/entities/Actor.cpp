// source/game/entities/Actor.cpp
#include "Actor.h"
#include "Collision3d.h"
#include "Deco3d.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "physics/physicsworld.h"
#include "physics/rigidbody.h"
#include "scene/scenemgr.h"
#include <cfloat>

jet::String Actor::k_tmplName("Actor");
const jet::String Actor::k_stateMachine("StateMachine");
const float Actor::k_defaultGravity = -1.5e-05f;
const float Actor::k_maxGravitySpeed = 1.0f;

// @0x59e9d0
Actor::Actor(const clara::Template *t)
    : Object(t), m_jump(0), m_jumpOrigin(0, 0, 0), m_jumpUp(0, 0, 0), m_gravityDir(0, 0, 0), m_gravity(0, 0, 0), m_groundCollision(0) {
    m_gravitySpeed[0] = m_gravitySpeed[1] = 0;
    m_type = 1;
}

// @0x59ea7c
Actor::~Actor() {}

// @0x59eb30: the gravity points down the scene's up vector
void Actor::Init() {
    Object::Init();
    const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
    m_gravity = math::vec3<float>(up.x * k_defaultGravity, up.y * k_defaultGravity, up.z * k_defaultGravity);
}

// @0x59eb8c: the data's state machine, then the actor starts it
void Actor::PostInit() {
    Object::PostInit();
    if (const clara::Param *p = FindParamByName(k_stateMachine)) m_stateMachine = p->GetAsState(0);
    InitStateMachine();
}

// @0x59ec10
void Actor::Update() {
    Object::Update();
    SM_Update();
}

// @0x59ec58: the set and state the data asks for, on the deco's model
void Actor::InitStateMachine() {
    m_initialState = m_stateMachine;
    SM_Init(GetDeco() ? GetDeco()->GetModel() : 0, m_initialState);
}

// @0x59ec94: the gravity along a direction (the scene's up vector when none), scaled
void Actor::ResetGravity(math::vec3<float> &dir, float scale) {
    m_gravityDir = dir;
    m_gravity = dir;
    if (m_gravity.getLength() == 0) m_gravity = jet::scene::SceneMgr::GetUpVector();
    m_gravity.normalize();
    m_gravitySpeed[0] = m_gravitySpeed[1] = 0;
    m_gravity = math::vec3<float>(m_gravity.x * k_defaultGravity * scale, m_gravity.y * k_defaultGravity * scale, m_gravity.z * k_defaultGravity * scale);
    SetCurrentJumpDefinition(0);
}

// @0x55b5ac
void Actor::ResetGravity(float scale) {
    static math::vec3<float> zero(0, 0, 0);
    ResetGravity(zero, scale);
}

// @0x59ed90: the gravity that lifts the actor by the height in the time, along the up vector; a timed jump also keeps the arc it follows
void Actor::ResetGravity(float timeMs, float height, float scale, bool timed) {
    const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
    if (!timed) m_gravitySpeed[0] = 0;
    float v = height * 1000.0f / timeMs * 1000.0f;
    const float g = scale * -8000000.0f / (v * v);
    v *= 0.0005f;
    const float speed = -(g * v);
    m_gravityDir = math::vec3<float>(speed * up.x, speed * up.y, speed * up.z);
    m_gravity = math::vec3<float>(g * up.x, g * up.y, g * up.z);
    if (timed) {
        m_gravitySpeed[1] = 0;
        m_gravitySpeed[0] = height / (timeMs * 0.001f);
        const math::vec3<float> p = GetPosition();
        m_jumpOrigin = math::vec3<float>(up.x * p.x, up.y * p.y, up.z * p.z);
        m_jumpUp = math::vec3<float>(scale * up.x, scale * up.y, scale * up.z);
    }
    SetCurrentJumpDefinition(0);
}

// @0x55b624
bool Actor::ProjectOnGround(math::vec3<float> &pos) {
    math::quat<float> rot(0, 0, 0, 1);
    return ProjectOnGround(pos, rot, 99999.0f, 1.0f);
}

// @0x55b65c
bool Actor::ProjectOnGround(math::vec3<float> &pos, const math::vec3<float> &dir, float distance) {
    math::vec3<float> back(-dir.x, -dir.y, -dir.z);
    back.normalize();
    const math::vec3<float> from = pos + math::vec3<float>(back.x * distance, back.y * distance, back.z * distance);
    const math::vec3<float> to = pos + dir;
    return ProjectOnGround(from, to, pos);
}

// @0x55b708: from the point raised by the last distance along the rotated up to the point lowered by the first
bool Actor::ProjectOnGround(math::vec3<float> &pos, math::quat<float> &rot, float below, float above) {
    const math::vec3<float> up = rot * jet::scene::SceneMgr::GetUpVector();
    const math::vec3<float> from = pos + math::vec3<float>(above * up.x, above * up.y, above * up.z);
    const math::vec3<float> to = pos + math::vec3<float>(-below * up.x, -below * up.y, -below * up.z);
    return ProjectOnGround(from, to, pos);
}

// @0x59f36c: the ray goes through the groups the actor's own collision is in and the masks it meets but a few (0x79 are taken out)
bool Actor::ProjectOnGround(const math::vec3<float> &from, const math::vec3<float> &to, math::vec3<float> &out) {
    static RaycastResult hit;
    hit.distance = FLT_MAX;
    unsigned short group = 5, mask = 2;  // the original's defaults for an actor without a collision (@0x59f36c: r3 = 5, r2 = 2)
    if (Collision3d *c = GetCollision()) {
        group = c->GetCollisionGroup();
        mask = c->GetCollisionMask() & ~0x79;
    }
    if (!GameLevel::GetInstance()->GetPhysicsWorld()->RayCast(from, to, group, mask, hit)) return false;
    out = hit.position;
    m_groundCollision = hit.bodyId ? static_cast<Collision3d*>(static_cast<CollisionBody*>(hit.bodyId)->GetUserData()) : 0;
    return true;
}

// @0x59eed0: the gravity speeds the fall (to a limit); a jump moves the actor along its path; a timed one along an arc; then
// what is below is looked for
bool Actor::UpdateGravity(math::vec3<float> &pos, bool project) {
    const float dt = Game::GetInstance()->GetUpdateTimeAsFloat();
    m_gravityDir += math::vec3<float>(m_gravity.x * dt, m_gravity.y * dt, m_gravity.z * dt);
    if (m_gravityDir.getLength() > k_maxGravitySpeed) {
        m_gravityDir.normalize();
        m_gravityDir = math::vec3<float>(m_gravityDir.x * k_maxGravitySpeed, m_gravityDir.y * k_maxGravitySpeed, m_gravityDir.z * k_maxGravitySpeed);
    }
    math::vec3<float> delta(m_gravityDir.x * dt, m_gravityDir.y * dt, m_gravityDir.z * dt);
    const math::vec3<float> gravity = m_gravity;
    math::vec3<float> gn = gravity, dn = delta;
    gn.normalize();
    dn.normalize();
    bool falling = math::dot(gn, dn) > 0;
    bool moved = true;
    if (m_jump) {
        if (m_jump->UpateControl(m_jumpCtrl, dt)) {
            SetCurrentJumpDefinition(0);
        } else {
            // @0x59f040: the path's step is scaled per axis by the gravity's unit direction (the .so normalizes its gravity copy at
            // sp+0x14 @0x59efd4 and reads it back at 0x59f050..0x59f070); the gravity's magnitude is not in it, else the arc is flat
            math::vec3<float> g2 = gravity;
            g2.normalize();
            const math::vec3<float> mv = m_jump->GetMovementDelta(m_jumpCtrl);
            delta = math::vec3<float>(g2.x * mv.x, g2.y * mv.y, g2.z * mv.z);
            math::vec3<float> n = delta;
            n.normalize();
            if (math::dot(g2, n) > 0) falling = true;
            else {
                m_gravityDir = math::vec3<float>(0, 0, 0);
                moved = false;
            }
        }
    } else if (m_gravitySpeed[0] > 0 && m_gravitySpeed[1] < m_gravitySpeed[0]) {
        m_gravitySpeed[1] += dt;
        if (m_gravitySpeed[1] >= m_gravitySpeed[0]) {
            m_gravitySpeed[1] = 0;
            m_gravitySpeed[0] = 0;
        } else {
            const float t = m_gravitySpeed[1] / m_gravitySpeed[0];
            float k;
            if (t > 0.7f) {
                const float s = 3.3333333f * (t - 0.7f);
                k = 1.0f - s * s;
                falling = true;
            } else {
                const float s = 1.0f - t * 1.4285715f;
                k = 1.0f - s * s;
                falling = false;
            }
            const math::vec3<float> &up = jet::scene::SceneMgr::GetUpVector();
            const math::vec3<float> lift(m_jumpUp.x * k, m_jumpUp.y * k, m_jumpUp.z * k);
            const math::vec3<float> target = m_jumpOrigin + lift;
            delta = target - math::vec3<float>(up.x * pos.x, up.y * pos.y, up.z * pos.z);
        }
    }
    (void)moved;
    if (falling) {
        if (!project) {
            pos += delta;
            return false;
        }
        // from a step back along the way it goes to where it gets
        math::vec3<float> back(-delta.x, -delta.y, -delta.z);
        back.normalize();
        math::vec3<float> hit = pos;
        if (ProjectOnGround(pos + back, pos + delta, hit)) {
            pos = hit;
            return true;
        }
        pos += delta;
        return false;
    }
    if (!project) {
        pos += delta;
        return false;
    }
    math::vec3<float> target = pos + delta;
    math::quat<float> rot(0, 0, 0, 1);
    const bool found = ProjectOnGround(target, rot, 99999.0f, 1.0f);
    // it is put on the ground only when it would go through it by more than a little
    if (found && pos.z - target.z < -0.1f) {
        pos = target;
        return true;
    }
    pos += delta;
    return false;
}

// @0x59f484: the gravity is the default one, the jump starts at the place from the beginning
void Actor::StartJumpDefinition(JumpDefinition *j, const math::vec3<float> &pos, const math::quat<float> &rot) {
    static math::vec3<float> zero(0, 0, 0);
    ResetGravity(zero, 1.0f);
    SetCurrentJumpDefinition(j);
    m_jumpCtrl = JumpDefinitionControl();
    if (!m_jump) return;
    SetTransform(pos, rot);
    m_jump->InitCtrl(m_jumpCtrl);
}
