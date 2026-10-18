// libs/glf/include/glf/input/inputmanager.h
#ifndef LIBS_GLF_INCLUDE_GLF_INPUT_INPUTMANAGER_H
#define LIBS_GLF_INCLUDE_GLF_INPUT_INPUTMANAGER_H

namespace glf {
class InputManager : public glf::EventReceiver {  // sizeof 0x28
public:
    InputManager();
    virtual ~InputManager();
    void UpdateFrame();
    void EnableAccelerometer(bool, float);
    bool EnableGyro(bool, float);
    bool IsAccelerometerEnabled();
    void EnableSensor(bool);
    glf::Keyboard &GetKeyboard(unsigned);
    unsigned GetKeyboardCount();
    unsigned GetKeyboardIndex(const glf::InputDevice*);
    glf::Mouse &GetMouse(unsigned);
    unsigned GetMouseCount();
    unsigned GetMouseIndex(const glf::InputDevice*);
    glf::Gamepad &GetGamepad(unsigned);
    unsigned GetGamepadCount();
    unsigned GetGamepadIndex(const glf::InputDevice*);
    void EmulateGamepadWin32(glf::GamepadType);
    virtual bool OnEvent(const glf::CoreEvent&);
    void StartDuoProtocol(bool);
private:
    void AddUpdatedDevice(glf::InputDevice*);
public:
    glf::InputManager::Impl &GetImpl();
private:
    std::set<glf::InputDevice *,std::less<glf::InputDevice *>,std::allocator<glf::InputDevice *> > updatedDevices;  // 0x8
    glf::InputManager::Impl &m;  // 0x24
};
}

#endif
