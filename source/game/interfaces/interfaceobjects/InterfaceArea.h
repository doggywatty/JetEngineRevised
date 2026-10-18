// source/game/interfaces/interfaceobjects/InterfaceArea.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEAREA_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEAREA_H

#include "InterfaceObject.h"

// An invisible rectangle: a place to anchor other elements to
class InterfaceArea : public InterfaceObject {  // sizeof 0x1A4
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceArea() {}
    InterfaceArea(AuroraLevelData *level, int id) { m_guiLevel = level; m_guiId = id; }
    virtual ~InterfaceArea() {}
    void Init(math::vec2<float> pos, math::vec2<float> size);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    static const jet::String k_uiTmplName;
};

#endif
