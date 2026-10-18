// source/game/interfaces/interfaceobjects/Interface3DCostumePos.cpp
#include "Interface3DCostumePos.h"
#include "game/GameLevel.h"
#include "game/dynamicobjects/EffectMgr.h"
#include "game/dynamicobjects/ModelPool.h"
#include "game/entities/Deco3d.h"
#include "game/entities/MinionCostume.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/utils/GameUtils.h"
#include "project.h"
#include "video/painter.h"

const jet::String Interface3DCostumePos::k_uiTmplName("3DCostumePos");
const jet::String Interface3DCostumePos::k_Minion_Costume_State_Name_Idle("Idle");

// @0x3be550: the costume is turned 10 degrees about x
const math::mat3<float> &Interface3DCostumePos::CostumeRotation() {
    static const math::mat3<float> m = math::quat<float>(-0.087155748f, 0.0f, 0.0f, 0.99619472f).getAsMat3();
    return m;
}

// @0x4c01a0
Interface3DCostumePos::Interface3DCostumePos() : m_minionCostume(0), m_renderShadowDeco(false), m_shadowDecoScale(0, 0, 0) {}

// @0x4c013c
Interface3DCostumePos::Interface3DCostumePos(AuroraLevelData *level, int id) : Interface3DCostumePos() {
    m_guiLevel = level;
    m_guiId = id;
}

// @0x4c01f0
Interface3DCostumePos::~Interface3DCostumePos() { _ClearMinionData(); }

// @0x4c0280: position 2 3, scale (percent) 10, the animixer state name 9; the equipped costume, with its shadow
void Interface3DCostumePos::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    jet::String state = AuroraLevelData::GetAuroraStringFromIndex(l->GetParamValue(e, 9));
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), (float)l->GetParamValue(e, 10) * 0.01f, state, jet::String::null, true);
}

// @0x4c0388
void Interface3DCostumePos::Init(math::vec2<float> pos, float scale, const jet::String &state, const jet::String &libName, bool shadow) {
    SetPos(pos);
    SetScale(math::vec2<float>(scale, scale));
    m_animixerStateName = state;
    m_minionCostumeLibName = libName;
    m_renderShadowDeco = shadow;
}

// @0x4c03f8: the costume copy is updated at the element's scale; its state sounds play only where it is on screen
void Interface3DCostumePos::Update(int dt) {
    if (!IsVisible()) return;
    if (IsActive()) {
        _CheckMinionReload();
        if (m_minionCostume) {
            const float s = Get3DAspectRatioScale() * GetCombinedScale().x;
            m_minionCostume->SetScale(math::vec3<float>(s, s, s));
            m_minionCostume->Update();
            m_minionCostume->GetDeco()->Update();
            const bool onScreen = IsScreenPosVisible(GetCombinedPos());
            m_minionCostume->SetTracksEnabled(onScreen, onScreen);
            EffectMgr::GetInstance()->Update(m_minionCostume->GetEffectLayerId(), dt);  // @0x4c053c
        }
    } else if (m_minionCostume)
        m_minionCostume->SetTracksEnabled(false, false);
    UpdateChildren(dt);
}

// @0x4c0584: the costume goes under the element's position, facing the camera plane (turned by the costume angle)
void Interface3DCostumePos::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    if (m_minionCostume) {
        painter.Flush();
        const float s = Get3DAspectRatioScale() * GetCombinedScale().x;
        math::vec2<float> p = GetPos(), o = GetOffset();
        math::vec3<float> at = MenuModel_Get3DPos(cam, math::vec2<float>(o.x + p.x, o.y + p.y));
        math::mat3<float> face = MenuModel_FaceCameraPlane(cam, at) * CostumeRotation();
        m_minionCostume->SetRotation(math::quat<float>(face));
        m_minionCostume->SetPosition(at);
        m_minionCostume->SetScale(math::vec3<float>(s, s, s));
        Deco3d *deco = m_minionCostume->GetDeco();
        deco->SetScale(math::vec3<float>(s, s, s));
        deco->Render();
        if (Deco3d *shadow = m_minionCostume->GetShadowDeco(); shadow && m_renderShadowDeco) {
            shadow->SetScale(math::vec3<float>(s * m_shadowDecoScale.x, s * m_shadowDecoScale.y, s * m_shadowDecoScale.z));
            shadow->Render();
        }
        // @0x4c078c: the effects of the costume (an element that is not active only follows the costume)
        const int layer = m_minionCostume->GetEffectLayerId();
        EffectMgr *effects = EffectMgr::GetInstance();
        if (!IsActive()) effects->Update(layer, 0);
        ModelPool::GetInstance()->Render(GameLevel::GetMenuSceneMgr(), layer);
        GameUtils::MenuRenderScene(GameLevel::GetMenuSceneMgr(), 0, effects->Particles_HasParticlesFor(layer), false);
        if (IsActive()) effects->Render(layer);
    }
    RenderChildren(cam, painter);
}

// @0x4c08a4: the costume copy is the one the element is for (the equipped one when it names none); a new one is made
// when that changes, in the state of the animixer name
void Interface3DCostumePos::_CheckMinionReload() {
    if (!clara::Project::GetInstance()) return;
    const jet::String &name = m_minionCostumeLibName.empty() ? CostumeMgr::GetInstance()->GetEquippedCostumeName() : m_minionCostumeLibName;
    if (m_minionCostume && m_minionCostume->GetClaraEntityName().Equals(name)) return;
    _ClearMinionData();
    MinionCostume *ref = dynamic_cast<MinionCostume*>(clara::Project::GetInstance()->FindEntityByName(name));
    if (!ref) ref = CostumeMgr::GetInstance()->GetPlaceHolderCostumeRef();
    m_minionCostume = new MinionCostume(ref, GameEntity::EGEConstructorFlagsDef::Interface);
    if (!m_minionCostume->CanLoadModel()) m_minionCostume->SM_SetAnimatedCostume(0);
    else m_minionCostume->SM_SetState(m_animixerStateName);
    m_minionCostume->RegisterForUpdate(false);
    Deco3d *deco = m_minionCostume->GetDeco();
    deco->SetSceneMgr(GameLevel::SceneMgr_Menu);
    deco->RegisterForUpdate(false);
    deco->RegisterForRender(false);
    if (Deco3d *shadow = m_minionCostume->GetShadowDeco()) {
        shadow->SetSceneMgr(GameLevel::SceneMgr_Menu);
        shadow->RegisterForUpdate(false);
        shadow->RegisterForRender(false);
        m_shadowDecoScale = shadow->GetScale();
    }
    deco->SetShadowEnabled(false);
}

// @0x4c0a28
void Interface3DCostumePos::_ClearMinionData() {
    if (m_minionCostume) {
        m_minionCostume->SM_StopAllSounds();
        delete m_minionCostume;
    }
    m_minionCostume = 0;
}

// @0x4c0ab0
void Interface3DCostumePos::SetAnimixerState(const jet::String &s) {
    m_animixerStateName = s;
    if (m_minionCostume) m_minionCostume->SM_SetState(s);
}

// @0x4c0ae8
void Interface3DCostumePos::OnMenuQuit() {
    if (m_minionCostume) m_minionCostume->SM_StopAllSounds();
}
