// libs/glf/include/glf/input/inputdevice.h
#ifndef LIBS_GLF_INCLUDE_GLF_INPUT_INPUTDEVICE_H
#define LIBS_GLF_INCLUDE_GLF_INPUT_INPUTDEVICE_H

namespace glf {
class InputDevice {  // sizeof 0x50
public:
    virtual const glf::Rtti &GetRtti();
    const char *GetRttiName();
    static const char *GetTypename();
    virtual void ResetFrame();
    glf::InputManager *GetInputManager();
    void SetInputManager(glf::InputManager*);
    void SetUpdated();
    void UpdateButton(unsigned, bool);
    void UpdateCursor(unsigned, bool, int, int);
    void UpdateStick(unsigned, float, float);
    void UpdateAcc(float, float, float);
    void UpdateGyro(float, float, float);
    void UpdateConnectionStatus(bool);
    const glf::vec3<float> &GetAcceleration();
    const glf::vec3<float> &GetGyro();
    const glf::vec3<float> &GetCompass();
    virtual glf::InputDevice *GetExtension();
    bool IsConnected();
    virtual bool PreSendEvent(glf::CoreEvent&);
protected:
    InputDevice();
    virtual ~InputDevice();
public:
    static const glf::Rtti sRtti;
    glf::InputDevice::InputPrimitiveArray<glf::SimpleButton> mBtns;  // 0x4
    glf::InputDevice::InputPrimitiveArray<glf::AnalogicStick> mAnalogicSticks;  // 0xC
    glf::InputDevice::InputPrimitiveArray<glf::Cursor> mCursors;  // 0x14
    glf::InputDevice::InputPrimitiveArray<glf::TouchPad> mTouchPads;  // 0x1C
private:
    glf::vec3<float> mCompass;  // 0x24
    glf::vec3<float> mAcc;  // 0x30
    glf::vec3<float> mGyro;  // 0x3C
    bool mConnected;  // 0x48
    glf::InputManager *mOverridenInputManager;  // 0x4C
};
}

#endif
