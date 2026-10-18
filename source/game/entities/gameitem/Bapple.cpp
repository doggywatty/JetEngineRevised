// source/game/entities/gameitem/Bapple.cpp
#include "Bapple.h"
#include "../Deco3d.h"
#include "param.h"
#include "project.h"

jet::String Bapple::k_tmplName("Bapple");
const jet::String Bapple::k_shopDecoParam("shopDeco"), Bapple::k_idleEffectParam("idleEffect");

// @0x5660fc
Bapple::Bapple(const clara::Template *t) : CollectableItem(t), m_shopDecoRef(0), m_idleEffect(0) { m_type = 4; }

// @0x56614c
Bapple::~Bapple() {}

// @0x5661ec
void Bapple::Init() { CollectableItem::Init(); }

// @0x5661fc
void Bapple::PostInit() {
    clara::Path path;
    if (const clara::Param *p = FindParamByName(k_shopDecoParam)) path = p->GetAsPath(0);
    m_shopDecoRef = dynamic_cast<Deco3d*>(clara::Project::GetInstance()->FindEntityByPath(path));
    if (const clara::Param *p = FindParamByName(k_idleEffectParam)) path = p->GetAsPath(0);
    m_idleEffect = clara::Project::GetInstance()->FindEntityByPath(path);
    if (m_flags & F_LIBRARY) {
        InitEffectOnContact();
        return;
    }
    RegisterForUpdate(true);
    RegisterForRender(false);
    CollectableItem::PostInit();
}

// @0x5661c0 (BappleMgr::OnBappleCollected joins with the manager)
void Bapple::Collect(Minion*) { m_state = CollectableStateDef::Collected; }
