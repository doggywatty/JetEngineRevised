// source/game/cameracontrollers/FluffyCamera.h
#ifndef SOURCE_GAME_CAMERACONTROLLERS_FLUFFYCAMERA_H
#define SOURCE_GAME_CAMERACONTROLLERS_FLUFFYCAMERA_H

class FluffyCamera : public CameraController {  // sizeof 0x114
public:
    FluffyCamera(CameraFluffyEntity::FluffyCameraConfigStruct*);
    void PostInit();
    void InitCameraPositionAndRotation();
    virtual math::vec3<float> GetPosition();
    virtual math::quat<float> GetRotation();
    virtual float GetFov();
    virtual const jet::String &GetType();
    virtual void Update(const unsigned);
    void UpdateTimers(const unsigned);
    virtual void Serialize(jet::stream::IStreamW&);
    virtual void Deserialize(jet::stream::IStream&);
    CameraFluffyEntity::FluffyCameraConfigStruct::FocusType GetFocusType();
private:
    void InitPosition();
    void SetPosition(const math::vec3<float>&);
    void SetRotation(const math::quat<float>&);
    void SetFov(float);
public:
    static jet::String k_cameraType;
private:
    math::vec3<float> m_position;  // 0x4
    math::quat<float> m_rotation;  // 0x10
    float m_fov;  // 0x20
    float m_posInitialized;  // 0x24
    math::vec3<float> m_initialPos;  // 0x28
    math::vec3<float> m_initialTargetPos;  // 0x34
    math::vec3<float> m_cameraLocalPosition;  // 0x40
    math::quat<float> m_cameraLocalRotation;  // 0x4C
    jet::core::Delayer<float,jet::core::LinearStep> m_targetZDelayer;  // 0x5C
    jet::core::Delayer<math::vec3<float>,jet::core::LinearStep> m_globalPositionDelayer;  // 0x74
    jet::core::Delayer<math::vec3<float>,jet::core::LinearStep> m_localPositionDelayer;  // 0xA4
    jet::core::Delayer<math::quat<float>,jet::core::LinearStep> m_rotationDelayer;  // 0xD4
    CameraFluffyEntity::FluffyCameraConfigStruct *m_fluffyCameraConfig;  // 0x110
    static const int k_cameraPositionDelayTime;
    static const int k_cameraRotationDelayTime;
};

#endif
