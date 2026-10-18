// source/game/interfaces/interfaceobjects/InterfaceDeviceSpecificAnim.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEDEVICESPECIFICANIM_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEDEVICESPECIFICANIM_H

#include "InterfaceObject.h"
#include "sprites/sprite.h"

// Plays the animation of a sprite that was drawn for the current screen size
class InterfaceDeviceSpecificAnim : public InterfaceObject {  // sizeof 0x1BC
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceDeviceSpecificAnim();
    InterfaceDeviceSpecificAnim(AuroraLevelData *level, int id);
    virtual ~InterfaceDeviceSpecificAnim();
    void Init(const math::vec2<float> &pos, int sprite, Sprite::SpriteBlending blending);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    static const jet::String k_uiTmplName;
private:
    void _ComputeAnim();
    math::rect<float> AnimRect(Sprite *s, int anim) const;
    int m_spriteIdx;  // 0x1A4
    Sprite::SpriteBlending m_blending;  // 0x1A8
    unsigned m_color;  // 0x1AC
    int m_anim;  // 0x1B0: -1 when none fits the screen
    int m_time;  // 0x1B4
    float m_scale;  // 0x1B8
};

#endif
