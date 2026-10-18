// source/game/entities/camera/CameraCinematicEntity.h
#ifndef SOURCE_GAME_ENTITIES_CAMERA_CAMERACINEMATICENTITY_H
#define SOURCE_GAME_ENTITIES_CAMERA_CAMERACINEMATICENTITY_H

#include "CameraEntity.h"
#include "camera/cinematiccamera.h"

class Deco3d;

// A camera animated inside a Deco3d's model (the CinematicDeco param)
class CameraCinematicEntity : public CameraEntity {  // sizeof 0x94
public:
    explicit CameraCinematicEntity(const clara::Template *t) : CameraEntity(t), m_deco(0), m_cinematicCamera(0) {}
    virtual ~CameraCinematicEntity() { delete m_cinematicCamera; }
    virtual void Init();
    virtual void PostInit();
    virtual CameraController *GetCameraController() { return m_cinematicCamera; }
    void SetDeco(Deco3d*);
    static jet::String k_tmplName;
private:
    Deco3d *m_deco;  // 0x8C
    CinematicCamera *m_cinematicCamera;  // 0x90
};

#endif
