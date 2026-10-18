// source/game/interfaces/interfaceobjects/InterfaceFillRect.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEFILLRECT_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEFILLRECT_H

#include "InterfaceObject.h"
#include "video/color.h"

// A rectangle of one colour, ARGB from parameters 11..14 of its Aurora element
class InterfaceFillRect : public InterfaceObject {  // sizeof 0x1A8
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceFillRect() : m_color(0xffffffffu) {}
    InterfaceFillRect(AuroraLevelData *level, int id) : m_color(0xffffffffu) { m_guiLevel = level; m_guiId = id; }
    virtual ~InterfaceFillRect() {}
    void Init(math::vec2<float> pos, math::vec2<float> size, unsigned argb);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetColor(unsigned argb) { m_color = argb; }
    static const jet::String k_uiTmplName;
private:
    unsigned m_color;  // 0x1A4: 0xAARRGGBB
};

#endif
