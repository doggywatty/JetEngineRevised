// source/game/entities/Achievement.cpp
#include "Achievement.h"
#include "AchievementSet.h"
#include "MissionCondition.h"
#include "Mission.h"
#include "game/Game.h"
#include "game/gamemanagers/AchievementsMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "param.h"
#include "project.h"
#include <cmath>

const jet::String Achievement::k_tmplName("Achievement");

// @0x59dda8
Achievement::Achievement(const clara::Template *t)
    : clara::DataEntity(t), m_location(0), m_id(0), m_dependency(0), m_completed(false), m_prize(0), m_set(0), m_shareInFacebook(false) {}

// @0x59dee8
void Achievement::Init() {
    clara::DataEntity::Init();
    m_nameCopy = GetName();
}

// @0x59df04
void Achievement::PostInit() {
    clara::DataEntity::PostInit();
    m_dependency = 0;
    if (const clara::Param *p = FindParamByName(jet::String("AchievementDepend"))) {
        const clara::Path &dependency = p->GetAsPath(0);
        if (!dependency.IsEmpty()) m_dependency = static_cast<Achievement*>(clara::Project::GetInstance()->FindEntityByPath(dependency));
    }
    GetParam(jet::String("shareInFacebook"), m_shareInFacebook);
    GetParam(jet::String("GameCenterId"), m_gameCenterId);
    GetParam(jet::String("ConditionsScope"), m_conditionsScope);
    m_location = FindParamByName(jet::String("ConditionsLocation"))->GetAsIdEnum(0).value;
    if (const clara::Param *p = FindParamByName(jet::String("AchievementPrize")))
        m_prize = static_cast<Price*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)));
    const clara::Param *conditions = FindParamByName(jet::String("AchievementConditions"));
    for (unsigned i = 0; conditions && i < conditions->GetComponentCount(); i++) {
        MissionCondition *c = dynamic_cast<MissionCondition*>(conditions->GetAsEntity(i));
        if (!c) continue;
        c->SetLocation(m_location);
        c->MissionLinkTo(this);
        m_conditions.push_back(c);
    }
    GetParam(jet::String("AchievementId"), m_id);
}

// @0x59e1dc: all the conditions hold: it is done
void Achievement::OnConditionCompleted() {
    bool all = true;
    for (size_t i = 0; i < m_conditions.size() && all; i++) all = m_conditions[i]->IsCompleted();
    m_completed = all;
    if (!all) return;
    if (m_set) m_set->OnAchievementCompleted(this);
    if (CanBeSeenByUser()) ShowIngameNotification();
    UnregisterObservers();
    // GameTrackingMgr::SendAchievementsUnlockedThroughGamePlayEvent(id, 0x1c61b, 0): tracking is not ported
}

// @0x59e288: its conditions start to count, from the career's statistics when that is its scope
void Achievement::OnStarted() {
    m_completed = false;
    const bool career = m_conditionsScope.Equals(Mission::k_missionScopeCareer);
    for (size_t i = 0; i < m_conditions.size(); i++) {
        m_conditions[i]->Reset();
        m_conditions[i]->SetStatType(career ? 1 : 0);
        m_conditions[i]->RegisterObserver();
    }
}

// @0x59e2ec: every condition that does not hold yet is forced (the ones that hold are skipped)
void Achievement::OnForceCompletion() {
    if (m_completed) return;
    for (size_t i = 0; i < m_conditions.size(); i++) {
        if (m_conditions[i]->IsCompleted()) continue;
        m_conditions[i]->OnForceCompletion();
    }
}

// @0x59e328: the text with the numbers of its conditions in it
const jet::String &Achievement::GetAchievementDescription() {
    GetParam(jet::String("AchievementDescription"), m_description);
    m_description = babel::Babel::GetInstance()->GetStringMgr()->Get(m_description).c_str();
    for (size_t i = 0; i < m_conditions.size(); i++) {
        if (m_conditions[i]->IsFloatValue())
            m_description = jet::String::Format(m_description.c_str(), (int)m_conditions[i]->GetFloatValue());
        else
            m_description = jet::String::Format(m_description.c_str(), m_conditions[i]->GetIntValue());
    }
    return m_description;
}

// @0x59e434
const jet::String &Achievement::GetAchievementName() {
    GetParam(jet::String("AchievementName"), m_name);
    m_name = babel::Babel::GetInstance()->GetStringMgr()->Get(m_name).c_str();
    return m_name;
}

// @0x59e4c0
void Achievement::MarkAsPreviouslyCompleted() {
    m_completed = true;
    UnregisterObservers();
}

// @0x59e4cc
void Achievement::UnregisterObservers() {
    for (size_t i = 0; i < m_conditions.size(); i++) m_conditions[i]->UnregisterObserver();
}

// @0x59e4f0: what the last condition has done, over the number of conditions (as the missions)
float Achievement::GetCompletedPercentage() {
    if (m_completed) return 1.0f;
    float p = 0;
    for (size_t i = 0; i < m_conditions.size(); i++) p = m_conditions[i]->GetCompletedPercentage();
    const float v = p / (float)m_conditions.size();
    return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}

// @0x59e564: how much of what its conditions ask is done
void Achievement::GetCompletedInt(int &done, int &total) {
    total = 0;
    done = 0;
    for (size_t i = 0; i < m_conditions.size(); i++) {
        const float percentage = m_conditions[i]->GetCompletedPercentage();
        const int value = m_conditions[i]->GetIntValue();
        total += value;
        done += (int)std::ceil((float)value * percentage);
    }
}

// @0x59e5f0
void Achievement::ForceConditionUpdate() {
    if (m_completed) return;
    for (size_t i = 0; i < m_conditions.size(); i++) m_conditions[i]->ForceUpdate();
}

// @0x59e628: not the Bapples one; its dependency done; and the social features on
bool Achievement::CanBeSeenByUser() {
    if (GetName().Equals("054_dm_Bapples")) return false;
    if (m_dependency && !m_dependency->m_completed) return false;
    return Game::GetInstance()->AreSocialFeaturesEnabled();
}

// @0x59e690: the notification in the game's interface (IGNotification_Achievement and IGNotificationMgr are not ported)
void Achievement::ShowIngameNotification() {}
