// source/game/interfaces/interfaceobjects/Interface3DElement.cpp
#include "Interface3DElement.h"
#include "game/GameLevel.h"
#include "game/entities/Deco3d.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/utils/GameUtils.h"
#include "project.h"
#include "scene/camera.h"
#include "video/driver.h"
#include "video/rendertarget.h"
#include <algorithm>

const jet::String Interface3DElement::k_uiTmplName("3DElement");

Interface3DElement::~Interface3DElement() { _Clear3DData(); }

// @0x449530: position from parameters 2 3, the clara node by name from 9, the scale from 0xb in percent
void Interface3DElement::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    float scale = (float)l->GetParamValue(e, 0xb) * 0.01f;
    jet::String node = l->GetAuroraStringFromIndex(l->GetParamValue(e, 9));
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), scale, node, false);
}

void Interface3DElement::Init(math::vec2<float> pos, float scale, const jet::String &claraNode, bool renderAtScreenPos) {
    SetPos(pos);
    SetScale(scale);
    m_claraNodeName = claraNode;
    m_renderAtScreenPos = renderAtScreenPos;
}

void Interface3DElement::_Clear3DData() {
    delete m_3DElement;
    m_3DElement = 0;
    m_3DElementRef = 0;
}

// _CheckReload3DElem @0x449ad0: the entity named by the element is copied once, a Deco3d into the menu scene
void Interface3DElement::_CheckReload3DElem() {
    clara::Project *project = clara::Project::GetInstance();
    if (!project || m_3DElement) return;
    _Clear3DData();
    clara::DataEntity *e = project->FindEntityByName(m_claraNodeName);
    if (!e) return;
    if (e->GetTemplateName().Equals(Deco3d::k_tmplName)) {
        m_3DElementRef = static_cast<clara::Entity*>(e);
        Deco3d *d = new Deco3d(static_cast<Deco3d*>(e), GameEntity::EGEConstructorFlagsDef::Interface);
        m_3DElement = d;
        d->SetSceneMgr(GameLevel::SceneMgr_Menu);
        d->RegisterForUpdate(false);
        d->RegisterForRender(false);
        return;
    }
    clara::Entity *ent = dynamic_cast<clara::Entity*>(e);
    m_3DElementRef = ent;
    m_3DElement = dynamic_cast<GameEntity*>(ent);
}

// @0x449680
void Interface3DElement::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) {
        _CheckReload3DElem();
        if (m_3DElement) m_3DElement->Update();
    }
    UpdateChildren(dt);
}

// @0x4496f0: the element's entity is moved under the element's screen position, facing the camera, and
// the menu scene is drawn clipped to the element's clip rectangle
void Interface3DElement::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    _CheckReload3DElem();
    if (m_3DElement) {
        painter.Flush();
        jet::video::Driver *drv = jet::video::Driver::GetCurrent();
        boost::shared_ptr<jet::video::RenderTarget> rt = drv->GetCrtRenderTarget();
        const bool clip = HasClip();
        if (clip) {
            math::rect<float> c = GetClip();
            const math::vec2<unsigned int> &sz = rt->GetSize();
            unsigned x0 = std::min((unsigned)std::max(0, (int)c.upperLeftCorner.x), sz.x);
            unsigned y0 = std::min((unsigned)std::max(0, (int)c.upperLeftCorner.y), sz.y);
            unsigned x1 = std::min((unsigned)std::max(0, (int)c.lowerRightCorner.x), sz.x);
            unsigned y1 = std::min((unsigned)std::max(0, (int)c.lowerRightCorner.y), sz.y);
            rt->SetScissorRect(math::rect<unsigned int>(x0, y0, x1, y1));
            rt->SetScissorTest(true);
        }
        if (m_renderAtScreenPos) {
            float scale = Get3DAspectRatioScale() * GetCombinedScale().x;
            math::vec2<float> pos = GetPos(), off = GetOffset();
            math::vec3<float> p = MenuModel_Get3DPos(cam, math::vec2<float>(off.x + pos.x, off.y + pos.y));
            const math::vec3<float> &rp = m_3DElementRef->GetPosition();
            math::vec3<float> at = p + math::vec3<float>(scale * rp.x, scale * rp.y, scale * rp.z);
            math::mat3<float> face = MenuModel_FaceCameraPlane(cam, at);
            math::mat3<float> rot = face * m_3DElementRef->GetRotation().getAsMat3();
            const math::vec3<float> &rs = m_3DElementRef->GetScale();
            m_3DElement->SetRotation(math::quat<float>(rot));
            m_3DElement->SetPosition(at);
            m_3DElement->SetScale(math::vec3<float>(scale * rs.x, scale * rs.y, scale * rs.z));
        }
        m_3DElement->Render();
        GameUtils::MenuRenderScene(GameLevel::GetMenuSceneMgr(), 0, false, false);
        drv->Flush();
        if (clip) {
            rt->SetScissorRect(math::rect<unsigned int>(0, 0, 0, 0));
            rt->SetScissorTest(false);
        }
    }
    RenderChildren(cam, painter);
}
