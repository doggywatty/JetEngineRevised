// source/game/entities/camera/CameraFluffyEntity.h
#ifndef SOURCE_GAME_ENTITIES_CAMERA_CAMERAFLUFFYENTITY_H
#define SOURCE_GAME_ENTITIES_CAMERA_CAMERAFLUFFYENTITY_H

class CameraFluffyEntity : public CameraEntity {  // sizeof 0xA0
protected:
    virtual bool RttiIsOfType(int);
    virtual void* RttiCastTo(int);
    static unsigned RttiGetClassId();
    static const char *RttiGetClassIdName();
    virtual const char *RttiGetInstanceClassIdName();
    virtual int RttiGetInstanceClassId();
public:
    virtual const jet::String &GetBaseTemplateName();
    CameraFluffyEntity(const clara::Template*);
    virtual ~CameraFluffyEntity();
    virtual void Init();
    virtual void PostInit();
    CameraFluffyEntity::FluffyCameraConfigStruct *GetCameraConfig();
    virtual CameraController *GetCameraController();
    static jet::String k_tmplName;
private:
    float m_fov;  // 0x90
    CameraTarget *m_focusTarget;  // 0x94
    FluffyCamera *m_followCamera;  // 0x98
    CameraFluffyEntity::FluffyCameraConfigStruct *m_camerConfig;  // 0x9C
};

#endif
