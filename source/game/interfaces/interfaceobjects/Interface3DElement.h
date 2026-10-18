// source/game/interfaces/interfaceobjects/Interface3DElement.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DELEMENT_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DELEMENT_H

#include "InterfaceObject.h"

class GameEntity;
class AuroraLevelData;
namespace clara { class Entity; }

// A copy of a clara entity of the menu library drawn in the menu scene where the element sits on screen
class Interface3DElement : public InterfaceObject {  // sizeof 0x1B4
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    Interface3DElement() : m_renderAtScreenPos(false), m_3DElement(0), m_3DElementRef(0) {}
    Interface3DElement(AuroraLevelData *level, int id) : m_renderAtScreenPos(false), m_3DElement(0), m_3DElementRef(0) { m_guiLevel = level; m_guiId = id; }
    virtual ~Interface3DElement();
    void Init(math::vec2<float> pos, float scale, const jet::String &claraNode, bool renderAtScreenPos);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset() { _CheckReload3DElem(); }
    virtual void UnloadClaraData() { _Clear3DData(); }
    virtual void SetScale(const math::vec2<float> &s) { SetScaleDirect(s); }
    virtual void SetScale(float s) { SetScaleDirect(math::vec2<float>(s, s)); }
    const jet::String &GetClaraNodeName() const { return m_claraNodeName; }
    GameEntity *Get3DElement() const { return m_3DElement; }
    static const jet::String k_uiTmplName;
private:
    void _CheckReload3DElem();
    void _Clear3DData();
    bool m_renderAtScreenPos;      // 0x1A4
    jet::String m_claraNodeName;   // 0x1A8
    GameEntity *m_3DElement;       // 0x1AC
    clara::Entity *m_3DElementRef; // 0x1B0
};

#endif
