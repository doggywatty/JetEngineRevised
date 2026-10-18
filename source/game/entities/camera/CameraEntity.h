// source/game/entities/camera/CameraEntity.h
#ifndef SOURCE_GAME_ENTITIES_CAMERA_CAMERAENTITY_H
#define SOURCE_GAME_ENTITIES_CAMERA_CAMERAENTITY_H

#include "game/entities/GameEntity.h"
#include "camera/cameracontroller.h"

// A level camera: the entity the Lua SetCamera hands to the camera manager through its controller
class CameraEntity : public GameEntity {  // sizeof 0x8C
public:
    explicit CameraEntity(const clara::Template *t) : GameEntity(t) {}
    virtual CameraController *GetCameraController() = 0;
};

#endif
