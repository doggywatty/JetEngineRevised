// source/game/interfaces/interfaceobjects/InterfaceSlide.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACESLIDE_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACESLIDE_H

#include "InterfaceObject.h"

// A slider: a track frame with a thumb that follows the touch along it (value 0..1)
class InterfaceSlide : public InterfaceObject {  // sizeof 0x1D8
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceSlide();
    InterfaceSlide(AuroraLevelData *level, int id);
    virtual ~InterfaceSlide();
    void Init(math::vec2<float> pos, int sprite, int trackFrame, int thumbFrame, int thumbPressedFrame);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset();
    float GetValue() { return m_value; }
    void SetValue(float v) { m_value = v; }
    bool IsPressed() { return m_state == 1; }
    static const jet::String k_uiTmplName;
private:
    void UpdateTouches();
    int m_state;  // 0x1A4: -1 untouched, 0 idle, 1 pressed
    int m_touchId;  // 0x1A8
    int m_spriteIdx;  // 0x1AC
    int m_trackFrame;  // 0x1B0
    int m_thumbFrame;  // 0x1B4
    int m_thumbPressedFrame;  // 0x1B8
    float m_value;  // 0x1BC
    math::vec2<float> m_thumbSize;  // 0x1C0
    math::rect<float> m_inputRect;  // 0x1C8
};

#endif
