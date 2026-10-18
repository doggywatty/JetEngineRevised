// source/game/interfaces/interfaceobjects/InterfacePBar.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEPBAR_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEPBAR_H

#include "InterfaceObject.h"
#include "sprites/spriteanim.h"

// A progress bar: up to three stacked fills over a background, with a marker at the end of the first; the fill types
// are the four directions of a rectangle, an animation stepped by the value, two radial bars and two needles
class InterfacePBar : public InterfaceObject {  // sizeof 0x234
public:
    enum BarType { LeftToRight = 0, RightToLeft = 1, TopToBottom = 2, BottomToTop = 3, FrameStepped = 4, RadialFill = 5, RadialEmpty = 6, NeedleUp = 7, NeedleDown = 8 };
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfacePBar();
    InterfacePBar(AuroraLevelData *level, int id);
    virtual ~InterfacePBar();
    void Init(int type, math::vec2<float> pos, int sprite, int bgAnim, int barAnim, int extra1Anim, int extra2Anim, int limitAnim, int logicRectIdx);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetValue(float);
    void SetExtra1Value(float v) { m_extra1Value = v; }
    void SetExtra2Value(float v) { m_extra2Value = v; }
    float GetValue() { return m_value; }
    float GetExtra1Value() { return m_extra1Value; }
    float GetExtra2Value() { return m_extra2Value; }
    void EnableInertia(bool on, float speed) { m_inertia = on; m_inertiaMultiPerMillisecond = speed; }
    void Blink() { m_blinkTimer = k_blink_time; }
    bool IsBlinking() { return m_blinkTimer > 0; }
    void UsePgtToBlinkColor(bool b) { m_usePgtToBlinkColor = b; }
    void EnableSinusBlink(bool b) { m_sinusBlink = b; }
    void SetBlinkColor(int c0, int c1) { m_blinkColor0 = c0; m_blinkColor1 = c1; }
    void SetAlpha(int a) { m_alpha = a; }
    void SetSmoothValue(bool s) { m_smoothValue = s; }
    static const jet::String k_uiTmplName;
    static const int k_blink_time = 500;
    static constexpr float k_defaultInertiaMultiPerMillisecond = 0.015f;
private:
    math::rect<float> GetPBarRect(float from, float to);
    math::vec2<float> GetLimitPos(float from, float to);
    bool m_firstValue;  // 0x1D4: no value set yet
    bool m_smoothValue;  // 0x1D5: the value moves to a new one over half a second
    float m_value;  // 0x1D8
    float m_valueStep;  // 0x1DC
    float m_valueCurrent;  // 0x1E0
    float m_extra1Value;  // 0x1E4
    float m_extra2Value;  // 0x1E8
    float m_valueToDraw;  // 0x1EC
    float m_extra1ValueToDraw;  // 0x1F0
    float m_extra2ValueToDraw;  // 0x1F4
    math::vec2<float> m_logicRectPos;  // 0x1F8
    bool m_inertia;  // 0x200: the drawn values chase the values
    float m_inertiaMultiPerMillisecond;  // 0x204
    int m_blinkTimer;  // 0x208
    bool m_usePgtToBlinkColor;  // 0x20C
    bool m_sinusBlink;  // 0x20D
    int m_blinkColor0;  // 0x210
    int m_blinkColor1;  // 0x214
    int m_alpha;  // 0x218
    int m_barType;  // 0x1A4
    int m_spriteIdx;  // 0x1A8
    int m_bgAnimId;  // 0x1AC
    int m_barAnimId;  // 0x1B0
    int m_extraBar1AnimId;  // 0x1B4
    int m_extraBar2AnimId;  // 0x1B8
    int m_limitAnimId;  // 0x1BC
    SpriteAnim *m_bgAnim;  // 0x1C0
    SpriteAnim *m_barAnim;  // 0x1C4
    SpriteAnim *m_extraBar1Anim;  // 0x1C8
    SpriteAnim *m_extraBar2Anim;  // 0x1CC
    SpriteAnim *m_limitAnim;  // 0x1D0
};

#endif
