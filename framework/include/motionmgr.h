// framework/include/motionmgr.h
#ifndef FRAMEWORK_INCLUDE_MOTIONMGR_H
#define FRAMEWORK_INCLUDE_MOTIONMGR_H

class MotionMgr {  // sizeof 0xBC
public:
    MotionMgr();
    ~MotionMgr();
    static MotionMgr *GetInstance();
    static bool HasInstance();
    static void FreeInstance();
    void Update();
    bool IsAccelerometerAvailable();
    bool IsGyroscopeAvailable();
    void AllowMotionDevice(bool);
    bool IsMotionDeviceAllowed();
    void EnableMotionDevice(bool);
    bool IsMotionDeviceActive();
    void CalibrateAccelVec();
    const math::quat<float> &GetGyroscopeQuat();
    const math::vec3<float> &GetReferenceAccelVec();
    const math::vec3<float> &GetDeviceRightVec();
    const math::vec3<float> &GetDeviceUpVec();
    const math::vec3<float> &GetDeviceFrontVec();
    void GetAccelerometerValues(math::vec3<float>&, math::vec3<float>&, math::vec3<float>&);
    void GetGyroscopeValues(float&, float&, float&, float&);
    void SetAccelerometerAvailable(bool);
    void SetGyroscopeAvailable(bool);
    void SetOrientation(safe_enum<jet::video::Display::OrientationDef,enum jet::video::Display::OrientationDef::type>);
    void DebugMovement();
private:
    void _ActivateDevice(bool);
    void _UpdateAccelerometerValues();
public:
    math::vec3<float> m_debugPos;  // 0x0
private:
    bool m_accelerometerAvailable;  // 0xC
    bool m_gyroscopeAvailable;  // 0xD
    math::quat<float> m_currentGyroQuat;  // 0x10
    math::quat<float> m_orientationQuat;  // 0x20
    math::vec3<float> m_referenceAccelVec;  // 0x30
    math::vec3<float> m_deviceRightVec;  // 0x3C
    math::vec3<float> m_deviceUpVec;  // 0x48
    math::vec3<float> m_deviceFrontVec;  // 0x54
    jet::core::Averager<MotionMgr::AccelVal> m_accelAvg;  // 0x60
    math::vec3<float> m_accelValueRaw;  // 0x78
    math::vec3<float> m_accelValueLow;  // 0x84
    math::vec3<float> m_accelValueHigh;  // 0x90
    bool m_enableDeviceMotion;  // 0x9C
    bool m_allowDeviceMotion;  // 0x9D
    jet::core::Averager<MotionMgr::AccelVal> m_calibrationAvg;  // 0xA0
    unsigned m_calibrationCounter;  // 0xB8
    static MotionMgr *s_instance;
};

#endif
