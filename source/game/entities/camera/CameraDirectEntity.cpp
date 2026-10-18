// source/game/entities/camera/CameraDirectEntity.cpp
#include "CameraDirectEntity.h"

jet::String CameraDirectEntity::k_tmplName("CameraDirect");

// @0x4bbb74
void CameraDirectEntity::Init() {
    GameEntity::Init();
    if (GetParam(jet::String("Fov"), m_fov)) {
        m_fov *= 0.017453292f;
        m_directCamera = new DirectCamera(GetPosition(), GetRotation(), m_fov);
    }
}

void CameraDirectEntity::SyncCamera() {
    if (!m_directCamera) return;
    m_directCamera->SetPosition(GetPosition());
    m_directCamera->SetRotation(GetRotation());
}

// @0x4bbc40
void CameraDirectEntity::Update() {
    GameEntity::Update();
    SyncCamera();
}

// @0x4bbc98
void CameraDirectEntity::NotifyTransformChanged(bool b) {
    GameEntity::NotifyTransformChanged(b);
    SyncCamera();
}
