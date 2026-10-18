// source/game/interfaces/interfaceobjects/Interface3DBodyPart.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DBODYPART_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DBODYPART_H

#include "InterfaceObject.h"

class AuroraLevelData;
class BodyPartOwner;

// A touch area that sits on a body part of its parent's 3D entity: it takes the transform of the part (or
// of a custom 3D layer) so its children are drawn on it
class Interface3DBodyPart : public InterfaceObject {  // sizeof 0x1E0
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    Interface3DBodyPart() : m_initialLayer3DId(0), m_layer3DId(0), m_modelBodyPart(0), m_useCustom3DLayerData(false) {}
    Interface3DBodyPart(AuroraLevelData *level, int id) : Interface3DBodyPart() { m_guiLevel = level; m_guiId = id; }
    virtual ~Interface3DBodyPart() {}
    void Init(math::vec2<float> pos, int w, int h, int layer3DId, int bodyPart);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset() {}
    void Layer3DData_SetCustom(const Interface3DLayer &l, bool on) { m_i3DLayerData = l; m_useCustom3DLayerData = on; }
    void Layer3DId_Set(int id) { m_layer3DId = id; }
    void Layer3DId_Reset() { m_layer3DId = m_initialLayer3DId; }
    void UpdateVisibilityFromBPO();
    static const jet::String k_uiTmplName;
private:
    BodyPartOwner *GetParentBodyPartOwner() const;
    int m_initialLayer3DId;           // 0x1A4
    int m_layer3DId;                  // 0x1A8
    int m_modelBodyPart;              // 0x1AC
    Interface3DLayer m_i3DLayerData;  // 0x1B0
    bool m_useCustom3DLayerData;      // 0x1DC
};

#endif
