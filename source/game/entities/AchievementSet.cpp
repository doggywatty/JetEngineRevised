// source/game/entities/AchievementSet.cpp
//
// Not ported: the publication of the completed achievements (OnlinePlayerData::PublishAchievementActivity).
#include "AchievementSet.h"
#include "Achievement.h"
#include "game/gamemanagers/AchievementsMgr.h"
#include "param.h"
#include "project.h"

const jet::String AchievementSet::k_tmplName("AchievementSet");
AchievementSet *AchievementSet::s_instance = 0;

// @0x59e6e0
AchievementSet::AchievementSet(const clara::Template *t) : GameEntity(t) { s_instance = this; }

// @0x59e7c8: the achievements of the "Achievements" paths know their set; the set is never updated nor drawn
void AchievementSet::PostInit() {
    GameEntity::PostInit();
    const clara::Param *list = FindParamByName(jet::String("Achievements"));
    for (unsigned i = 0; list && i < list->GetComponentCount(); i++) {
        Achievement *a = static_cast<Achievement*>(clara::Project::GetInstance()->FindEntityByPath(list->GetAsPath(i)));
        a->SetSet(this);
        m_achievements.push_back(a);
    }
    RegisterForUpdate(false);
    RegisterForRender(false);
}

// @0x59e924
void AchievementSet::OnAchievementCompleted(Achievement *a) {
    AchievementsMgr::GetInstance()->OnAchievementJustCompleted(a->GetName());
    // if (shareInFacebook) OnlinePlayerData::PublishAchievementActivity(name, gameCenterId, true): online services not ported
}
