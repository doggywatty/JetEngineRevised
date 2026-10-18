// source/game/interfaces/interfaceobjects/Interface3DTouchInteract.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DTOUCHINTERACT_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DTOUCHINTERACT_H

#include "InterfaceObject.h"

class TouchInteract3d;
class AuroraLevelData;
namespace clara { class Entity; }

// A touchInteract3d entity of the menu library, copied into the menu scene under the element's position
// (3D element drawing; the effect of a touch is started by TouchInteract3d::BPO_OnBodyPartTouched, the model pool is not ported yet)
class Interface3DTouchInteract : public InterfaceObject {  // sizeof 0x1B0
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    Interface3DTouchInteract() : m_touchInteractEntityRef(0), m_touchInteractEntity(0) {}
    Interface3DTouchInteract(AuroraLevelData *level, int id) : m_touchInteractEntityRef(0), m_touchInteractEntity(0) { m_guiLevel = level; m_guiId = id; }
    virtual ~Interface3DTouchInteract() {}
    void Init(math::vec2<float> pos, const jet::String &claraNode);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void UnloadClaraData() { _Clear3DData(); }
    virtual void Reset();
    virtual void OnMenuQuit();
    const jet::String &GetClaraNodeName() const { return m_claraNodeName; }
    TouchInteract3d *GetTouchInteractEntity() const { return m_touchInteractEntity; }
    void SetModelNodesVisible(const jet::String &prefix, bool);
    void SetBPSVisibleIfItHas(int event, bool);
    static const jet::String k_uiTmplName;
private:
    void _ResetTouchInteract3d();
    void _Clear3DData();
    jet::String m_claraNodeName;               // 0x1A4
    clara::Entity *m_touchInteractEntityRef;   // 0x1A8
    TouchInteract3d *m_touchInteractEntity;    // 0x1AC
};

#endif
