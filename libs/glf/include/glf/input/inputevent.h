// libs/glf/include/glf/input/inputevent.h
#ifndef LIBS_GLF_INCLUDE_GLF_INPUT_INPUTEVENT_H
#define LIBS_GLF_INCLUDE_GLF_INPUT_INPUTEVENT_H

namespace glf {
struct InputEvent : public glf::CoreEvent {  // sizeof 0x10
    InputEvent(int);
    glf::InputDevice *mInputDevice;  // 0x4
    __int64 mTimeStamp;  // 0x8
};
}

#endif
