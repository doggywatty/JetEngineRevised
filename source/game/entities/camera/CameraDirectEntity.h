// source/game/entities/camera/CameraDirectEntity.h
#ifndef SOURCE_GAME_ENTITIES_CAMERA_CAMERADIRECTENTITY_H
#define SOURCE_GAME_ENTITIES_CAMERA_CAMERADIRECTENTITY_H

#include "CameraEntity.h"
#include "camera/directcamera.h"

// A fixed camera at the entity's transform with its Fov param (degrees in the data)
class CameraDirectEntity : public CameraEntity {  // sizeof 0x94
public:
    explicit CameraDirectEntity(const clara::Template *t) : CameraEntity(t), m_fov(0), m_directCamera(0) {}
    virtual ~CameraDirectEntity() { delete m_directCamera; }
    virtual void Init();
    virtual void Update();
    virtual CameraController *GetCameraController() { return m_directCamera; }
    virtual void NotifyTransformChanged(bool);
    static jet::String k_tmplName;
private:
    void SyncCamera();
    float m_fov;  // 0x8C
    DirectCamera *m_directCamera;  // 0x90
};

#endif
