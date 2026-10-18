// source/game/interfaces/interfaceobjects/Interface3DTouchInteract.cpp
#include "Interface3DTouchInteract.h"
#include "Interface3DBodyPart.h"
#include "game/GameLevel.h"
#include "game/entities/Deco3d.h"
#include "game/entities/TouchInteract3d.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/utils/GameUtils.h"
#include "project.h"

const jet::String Interface3DTouchInteract::k_uiTmplName("3DTouchInteract");

// @0x449d38: position from parameters 2 3, the clara node by name from 9
void Interface3DTouchInteract::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    jet::String node = l->GetAuroraStringFromIndex(l->GetParamValue(e, 9));
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), node);
}

void Interface3DTouchInteract::Init(math::vec2<float> pos, const jet::String &claraNode) {
    SetPos(pos);
    m_claraNodeName = claraNode;
}

void Interface3DTouchInteract::_Clear3DData() {
    delete m_touchInteractEntity;
    m_touchInteractEntity = 0;
    m_touchInteractEntityRef = 0;
}

// _ResetTouchInteract3d @0x44a0cc: the entity named by the element is copied once into the menu scene
void Interface3DTouchInteract::_ResetTouchInteract3d() {
    if (m_touchInteractEntity || m_claraNodeName.empty()) return;
    _Clear3DData();
    m_touchInteractEntityRef = dynamic_cast<clara::Entity*>(clara::Project::GetInstance()->FindEntityByName(m_claraNodeName));
    m_touchInteractEntity = new TouchInteract3d(static_cast<TouchInteract3d*>(m_touchInteractEntityRef), GameEntity::EGEConstructorFlagsDef::Interface);
    TouchInteract3d *t = m_touchInteractEntity;
    t->SetClassType((BodyPartOwner::BodyPartOwnerClassType)8);  // @0x4c1820: the menu scene's class, the one the menus' touches look for
    t->SetEnabled(true);
    t->SetVisible(true);
    t->Reset(true);
    t->RegisterForUpdate(false);
    t->RegisterForRender(false);
    t->SetSceneMgr(GameLevel::SceneMgr_Menu);
}

void Interface3DTouchInteract::Reset() {
    _ResetTouchInteract3d();
    if (m_touchInteractEntity) m_touchInteractEntity->Reset(true);
}

void Interface3DTouchInteract::OnMenuQuit() {
    if (m_touchInteractEntity) m_touchInteractEntity->Reset(false);
}

// @0x449e24
void Interface3DTouchInteract::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive() && m_touchInteractEntity) m_touchInteractEntity->Update();
    UpdateChildren(dt);
}

// @0x449ef0: the entity goes under the element's screen position at the entity's depth in the menu
void Interface3DTouchInteract::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    if (m_touchInteractEntity) {
        painter.Flush();
        math::vec2<float> pos = GetCombinedPos();
        math::vec3<float> at = MenuModel_Get3DPos(cam, pos, m_touchInteractEntity->GetPosition());
        const math::vec3<float> &s = m_touchInteractEntityRef->GetScale();
        float k = Get3DAspectRatioScale();
        m_touchInteractEntity->SetPosition(at);
        m_touchInteractEntity->SetScale(math::vec3<float>(s.x * k, s.y * k, s.z * k));
        m_touchInteractEntity->ScaleUIDecoAndBP(k);
        m_touchInteractEntity->Render();
        GameUtils::MenuRenderScene(GameLevel::GetMenuSceneMgr(), 0, false, false);
    }
    RenderChildren(cam, painter);
}

void Interface3DTouchInteract::SetModelNodesVisible(const jet::String &prefix, bool v) {
    if (m_touchInteractEntity) m_touchInteractEntity->SetModelNodesVisible(prefix, v);
}

// @0x44a1e8: the body part elements follow the visibility of the parts
void Interface3DTouchInteract::SetBPSVisibleIfItHas(int event, bool visible) {
    if (!m_touchInteractEntity) return;
    m_touchInteractEntity->SetBPSVisibleIfItHas(event, visible);
    for (unsigned i = 0; i < GetChildCount(); i++) {
        InterfaceObject *c = GetChild((int)i);
        if (c->GetUITemplateName().Equals(Interface3DBodyPart::k_uiTmplName)) static_cast<Interface3DBodyPart*>(c)->UpdateVisibilityFromBPO();
    }
}
