// source/game/gamemanagers/SkyDomeMgr.cpp
#include "SkyDomeMgr.h"
#include "../GameLevel.h"
#include "../entities/Deco3d.h"
#include "../entities/Minion.h"

// @0x48bb20: the copy is linked to the minion while it stands at the height 0
void SkyDomeMgr::AddSkyDome(const Deco3d &deco) {
    if (m_name == deco.GetName()) return;
    CleanOldSkyDome();
    m_name = deco.GetName();
    m_dome = new Deco3d(const_cast<Deco3d*>(&deco), 0);
    Minion *m = GameLevel::GetInstance()->GetMinion();
    const math::vec3<float> pos = m->GetPosition();
    m->SetPosition(math::vec3<float>(pos.x, pos.y, 0));
    m_dome->LinkTo(m, jet::String::null, 0x17);
    m->SetPosition(pos);
}

// @0x48bc64
void SkyDomeMgr::CleanOldSkyDome() {
    if (!m_dome) return;
    delete m_dome;
    m_dome = 0;
    GameLevel::GetInstance()->RemoveEntities();
    m_name = jet::String::null;
}
