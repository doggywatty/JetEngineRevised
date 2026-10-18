// framework/include/camera/cameramgr.h
#ifndef FRAMEWORK_INCLUDE_CAMERA_CAMERAMGR_H
#define FRAMEWORK_INCLUDE_CAMERA_CAMERAMGR_H

#include <memory>
#include <vector>
#include "jsingleton.h"
#include "scene/camera.h"
#include "camera/cameracontroller.h"
#include "movements/shakemovement.h"
#include "movements/swaymovement.h"

enum SwayType { SWAY_NONE, SWAY_SLOW, SWAY_MEDIUM, SWAY_FAST };

class CameraMgrObserver {
public:
    virtual ~CameraMgrObserver() {}
    virtual void OnCameraChanged(CameraController*) = 0;
};

// Drives the scene camera from a stack of controllers, each blending in over its transition time
class CameraMgr : public Singleton<CameraMgr> {  // sizeof 0x58
public:
    explicit CameraMgr(jet::scene::Camera*);
    virtual ~CameraMgr();
    void Update(unsigned dt);
    // makes the controller current, blending in over blendMs after delayMs
    void SetCamera(CameraController*, unsigned blendMs, unsigned delayMs = 0);
    void ResetToCamera(CameraController*, unsigned blendMs);
    CameraController *GetCurrentController() { return m_cameras.empty() ? nullptr : m_cameras.back().controller; }
    void ShakeCamera(float, float, float, float, unsigned) {}  // @0x5aadf4: does nothing in the shipped game
    bool IsCameraShaking() const { return m_shake && (m_shake->m_hMax != 0 || m_shake->m_vMax != 0); }
    void SwayCamera(float yaw, SwayType type, float strength, int ms);
    void ClearAllCameras();
    jet::scene::Camera *GetSceneCamera() { return m_sceneCamera; }
    void SetPaused(bool p) { m_paused = p; }
    bool IsPaused() const { return m_paused; }
    void AddObserver(CameraMgrObserver*);
    void RemoveObserver(CameraMgrObserver*);
    unsigned GetCameraCount() { return (unsigned)m_cameras.size(); }
private:
    struct ControllerHolder { CameraController *controller; unsigned total, elapsed; };
    struct SetsCameraDelayer { CameraController *controller; unsigned delay, blend; };
    ShakeMovement *m_shake;  // 0x4
    SwayMovement *m_sway;  // 0x8
    std::vector<ControllerHolder> m_cameras;  // 0xC
    jet::scene::Camera *m_sceneCamera;  // 0x18
    std::shared_ptr<CameraController> m_dummyController;  // 0x1C
    bool m_paused;  // 0x24
    math::quat<float> m_deltaRot;  // 0x28
    std::vector<CameraMgrObserver*> m_observers;  // 0x38
    std::vector<SetsCameraDelayer> m_delayed;  // 0x48
    unsigned m_delayedTime;  // 0x54
};

#endif
