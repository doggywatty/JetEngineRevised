// source/game/entities/ShadowLightPoint.cpp
#include "ShadowLightPoint.h"
#include "game/Game.h"
#include "scene/camera.h"
#include "scene/model.h"
#include <cmath>

jet::String ShadowLightPoint::k_tmplName("ShadowLightPoint");
jet::scene::Model *ShadowLightPoint::m_modelRefPoint = 0;
bool ShadowLightPoint::m_canUseFirstSkeletalChild = true;

// @0x521a00
ShadowLightPoint::ShadowLightPoint(const clara::Template *t) : GameEntity(t) {}

// @0x521a4c
ShadowLightPoint::~ShadowLightPoint() {}

// @0x521ad4
void ShadowLightPoint::Init() {
    GameEntity::Init();
    RegisterForRender(true);
}

// @0x521af0, @0x521af4, @0x521b44: the entity does not follow enabling, visibility or moves
void ShadowLightPoint::SetEnabled(bool) {}
void ShadowLightPoint::SetVisible(bool) {}
void ShadowLightPoint::NotifyTransformChanged(bool) {}

// @0x521af8: with shadows on, aim the shadow camera from this entity's position
void ShadowLightPoint::Render() {
    GameEntity::Render();
    if (CGameConfig::Current().useShadows <= 0) return;
    ApplyRefPoint(GetPosition());
}

// @0x521b48: the shadow camera sits 10 units from the model's reference node, toward the light, and looks at it
// (up = +Z); the rotation is the quaternion of that basis
void ShadowLightPoint::ApplyRefPoint(const math::vec3<float> &lightPos) {
    if (!m_modelRefPoint) return;
    const jet::scene::Node *node = m_modelRefPoint->GetRootNode().GetChild(0);
    if (!m_canUseFirstSkeletalChild || !node || !(node->GetFlags() & jet::scene::ModelBase::NodeFlags::Skeletal))
        node = &m_modelRefPoint->GetRootNode();
    math::vec3<float> target = node->GetAbsolutePosition();

    math::vec3<float> dir = target - lightPos;
    dir.normalize();
    math::vec3<float> d10(dir.x * 10.0f, dir.y * 10.0f, dir.z * 10.0f);
    math::vec3<float> eye = target - d10;
    dir.normalize();
    math::vec3<float> f(-dir.x, -dir.y, -dir.z);
    math::vec3<float> up(0, 0, 1);
    up.normalize();
    math::vec3<float> r = math::cross(f, up);
    r.normalize();
    math::vec3<float> u = math::cross(r, f);

    // the 3x3 basis as rows: right, front, up
    float a[9] = { r.x, r.y, r.z, f.x, f.y, f.z, u.x, u.y, u.z };
    float q[4];  // x, y, z, w
    float trace = a[4] + a[0] + a[8];
    if (trace > 0) {
        float s = sqrtf(trace + 1.0f);
        float inv = 0.5f / s;
        q[3] = s * 0.5f;
        q[0] = (a[7] - a[5]) * inv;
        q[1] = (a[2] - a[6]) * inv;
        q[2] = (a[3] - a[1]) * inv;
    } else {
        int i = a[4] > a[0] ? 1 : 0;
        if (!(a[8] <= a[i * 4])) i = 2;
        int j = (i + 1) % 3, k = (j + 1) % 3;
        float s = sqrtf(a[i * 4] + 1.0f - a[j * 4] - a[k * 4]);
        float inv = 0.5f / s;
        q[i] = s * 0.5f;
        q[3] = (a[k * 3 + j] - a[j * 3 + k]) * inv;
        q[j] = (a[j * 3 + i] + a[i * 3 + j]) * inv;
        q[k] = (a[k * 3 + i] + a[i * 3 + k]) * inv;
    }

    jet::scene::Camera *cam = Game::GetInstance()->GetShadowCamera();
    if (eye != cam->GetPosition()) cam->SetPosition(eye);
    math::quat<float> rot(q[0], q[1], q[2], q[3]);
    if (!(cam->GetRotation() == rot)) cam->SetRotation(rot);
}
