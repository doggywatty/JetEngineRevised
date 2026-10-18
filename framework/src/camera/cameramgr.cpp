// framework/src/camera/cameramgr.cpp
#include "camera/cameramgr.h"
#include "camera/directcamera.h"
#include <algorithm>

// @0x5aa57c
CameraMgr::CameraMgr(jet::scene::Camera *c)
    : m_shake(0), m_sway(0), m_sceneCamera(c), m_paused(false), m_deltaRot(0, 0, 0, 1), m_delayedTime(0) {}

// @0x5aa650
CameraMgr::~CameraMgr() { ClearAllCameras(); }

// @0x5aac2c
void CameraMgr::ClearAllCameras() {
    m_cameras.clear();
    delete m_shake;
    m_shake = 0;
    delete m_sway;
    m_sway = 0;
}

// @0x5aa740: blends the stacked controllers (smoother step) into the scene camera
void CameraMgr::Update(unsigned dt) {
    if (m_paused) return;
    math::vec3<float> pos(0, 0, 0);
    math::quat<float> rot(0, 0, 0, 1);
    float fov = 0;
    if (!m_delayed.empty()) {
        m_delayedTime += dt;
        if (m_delayed.front().delay <= m_delayedTime) {
            SetsCameraDelayer d = m_delayed.front();
            SetCamera(d.controller, d.blend, 0);
            m_delayed.erase(m_delayed.begin());
            m_delayedTime = 0;
        }
    }
    for (size_t i = 0; i < m_cameras.size(); i++) {
        ControllerHolder &h = m_cameras[i];
        h.elapsed += dt;
        if (h.total < h.elapsed) h.elapsed = h.total;
        h.controller->Update(dt);
        math::vec3<float> p = h.controller->GetPosition();
        math::quat<float> r = h.controller->GetRotation();
        float f = h.controller->GetFov();
        if (i == 0 || h.total == 0) {
            pos = p;
            rot = r;
            fov = f;
        } else {
            float t = (float)h.elapsed / (float)h.total;
            float s = t * t * t * ((t * 6.0f - 15.0f) * t + 10.0f);
            if (s != s) s = 0;
            if (s >= 1.0f) s = 1.0f;
            pos = pos * (1.0f - s) + p * s;
            rot = math::lerp(rot, r, s);
            fov = fov + s * (f - fov);
        }
    }
    // a finished blend drops the controller under it
    for (size_t i = 1; m_cameras.size() > 1 && i < m_cameras.size();) {
        if (m_cameras[i].elapsed < m_cameras[i].total)
            i++;
        else
            m_cameras.erase(m_cameras.begin() + (i - 1));
    }
    if (m_shake) {
        m_shake->Update((int)dt);
        if (m_shake->m_hMax == 0.0f && m_shake->m_vMax == 0.0f) {
            delete m_shake;
            m_shake = 0;
        } else
            rot = m_shake->GetRotation() * rot;
    }
    if (m_sway) {
        m_sway->Render((int)dt);
        if (!m_sway->IsMoving()) {
            delete m_sway;
            m_sway = 0;
        } else {
            pos = pos + m_sway->GetPosition();
            rot = m_sway->GetRotation() * rot;
        }
    }
    m_sceneCamera->SetFov(fov);
    m_sceneCamera->SetTransform(pos, rot * m_deltaRot);
}

// @0x5aac5c
void CameraMgr::SetCamera(CameraController *c, unsigned blendMs, unsigned delayMs) {
    if (!c) return;
    if (!m_cameras.empty() && m_cameras.back().controller == c) return;
    if (delayMs) {
        m_delayed.push_back({ c, delayMs, blendMs });
        return;
    }
    if (m_cameras.size() < 10)
        m_cameras.push_back({ c, blendMs, 0 });
    else
        m_cameras.back() = { c, blendMs, 0 };
    for (CameraMgrObserver *o : m_observers) o->OnCameraChanged(c);
}

// @0x5aad88: starts over from where the scene camera is now
void CameraMgr::ResetToCamera(CameraController *c, unsigned blendMs) {
    if (!c) return;
    ClearAllCameras();
    m_dummyController.reset(new DirectCamera(m_sceneCamera->GetPosition(), m_sceneCamera->GetRotation(), m_sceneCamera->GetFov()));
    SetCamera(m_dummyController.get(), blendMs, 0);
}

// @0x5aae34
void CameraMgr::SwayCamera(float yaw, SwayType type, float strength, int ms) {
    if (!m_sway) m_sway = new SwayMovement;
    if (type == SWAY_NONE) {
        m_sway->Stop(ms);
        return;
    }
    m_sway->Start(yaw, (int)type, strength, ms);
}

// @0x5aaed4
void CameraMgr::AddObserver(CameraMgrObserver *o) {
    if (std::find(m_observers.begin(), m_observers.end(), o) == m_observers.end()) m_observers.push_back(o);
}

// @0x5aaf6c
void CameraMgr::RemoveObserver(CameraMgrObserver *o) {
    std::vector<CameraMgrObserver*>::iterator it = std::find(m_observers.begin(), m_observers.end(), o);
    if (it != m_observers.end()) m_observers.erase(it);
}
