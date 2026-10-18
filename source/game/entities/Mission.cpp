// source/game/entities/Mission.cpp
//
// Not ported (see Mission.h): the in-game notification, the tracking events and the GUI objects of the first time.
#include "Mission.h"
#include "MissionCondition.h"
#include "game/Game.h"
#include "game/Player.h"
#include "game/SortPageHelp.h"
#include "game/Statistics.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"
#include "project.h"

const jet::String Mission::k_tmplName("Mission");  // @0xc41f1d
const jet::String Mission::k_missionScopeCurrentRun("CurrentRun");
const jet::String Mission::k_missionScopeCareer("Career");
const jet::String Mission::k_missionScopeAccumulative("Accumulative");
const jet::String Mission::k_missionGiver_Gru("Gru");
const jet::String Mission::k_missionGiver_Nefario("Nefario");
const jet::String Mission::k_missionGiver_Girls("Girls");
const jet::String Mission::k_guiAssociated("MissionPopup");
const jet::String Mission::k_hasPopup("MissionHasPopup");

// @0x57c870
Mission::Mission(const clara::Template *t)
    : clara::DataEntity(t), m_missionId(0), m_version(0), m_location(0), m_activeMissionSlot(0), m_completed(false), m_isActive(true), m_skipPrice(0),
      m_missionMultiplierPoints(0), m_hasPopup(false), m_hasDependency(false), m_isMandatory(false) {}

// @0x57c96c
Mission::~Mission() {
    for (size_t i = 0; i < m_conditions.size(); i++) m_conditions[i]->MissionLinkTo(0);
}

// @0x57ca28: the data of the mission and its conditions, which know it is theirs
void Mission::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(jet::String("IsMissionActive"), m_isActive);
    int points = 0;
    GetParam(jet::String("MissionMultiplierPoints"), points);
    m_missionMultiplierPoints = (unsigned)points;
    GetParam(jet::String("MissionGiver"), m_missionGiver);
    if (const clara::Param *loc = FindParamByName(jet::String("MissionLocation"))) m_location = loc->GetAsIdEnum(0).value;
    if (const clara::Param *price = FindParamByName(jet::String("SkipPrice")))
        m_skipPrice = dynamic_cast<Price*>(clara::Project::GetInstance()->FindEntityByPath(price->GetAsPath(0)));
    GetParam(jet::String("MissionScope"), m_scope);
    if (const clara::Param *conditions = FindParamByName(jet::String("MissionContitions"))) {
        for (unsigned i = 0; i < conditions->GetComponentCount(); i++) {
            MissionCondition *c = dynamic_cast<MissionCondition*>(conditions->GetAsEntity(i));
            if (!c) continue;
            c->SetLocation(m_location);
            c->MissionLinkTo(this);
            m_conditions.push_back(c);
        }
    }
    GetParam(jet::String("MissionId"), m_missionId);
    GetParam(jet::String("MissionId"), m_missionIdString);
    int version = 0;
    GetParam(jet::String("MissionVersion"), version);
    m_version = (unsigned)version;
    GetParam(k_hasPopup, m_hasPopup);
    if (m_hasPopup) {
        // the GUI objects of the popup (the .so finds them by the paths of "MissionPopup"): GuiObject is not ported
        TutorialMgr::GetInstance()->RegisterMission((unsigned)m_missionId);
    }
    GetParam(jet::String("MissionHasDependency"), m_hasDependency);
    if (m_hasDependency) GetParam(jet::String("DependencyMissionId"), m_dependencyMissionId);
    GetParam(jet::String("IsMissionMandatory"), m_isMandatory);
}

// @0x57ced4: all the conditions hold: the mission is done
void Mission::OnConditionCompleted() {
    bool all = true;
    for (size_t i = 0; i < m_conditions.size() && all; i++) all = m_conditions[i]->IsCompleted();
    m_completed = all;
    if (!all) return;
    MissionMgr::GetInstance()->OnMissionJustCompleted(GetName(), 0);
    UnregisterObservers();
    // GameTrackingMgr::SendMissionInteractionsEvent
    ShowIngameNotification();
    SortPageHelp::GetInstance()->m_missionDone = true;
}

// @0x57cfc0: the notification of the mission in the game's interface, and the first mission marks the player's progress
// (IGNotification_Mission and GS_Gameplay's interface are not ported)
void Mission::ShowIngameNotification() {}

// @0x57d09c
void Mission::OnMarkAsPreviouslyCompleted() {
    if (m_completed) return;
    m_completed = true;
    UnregisterObservers();
}

// @0x57d0b4: skipped (paid for): the tokens it cost are told
void Mission::OnForceCompletion() {
    if (m_completed) return;
    m_completed = true;
    int paid = 0;
    if (m_skipPrice->GetPriceType() == Price::Tokens) paid = m_skipPrice->GetAmount();
    MissionMgr::GetInstance()->OnMissionJustCompleted(GetName(), paid);
    UnregisterObservers();
    // GameTrackingMgr::SendMissionInteractionsEvent
}

// @0x57d180: it takes a slot of the active missions: its conditions start to count, from what the statistics of its type are
void Mission::OnAddedToActiveMission(int slot, int repeatCount, bool reset) {
    m_activeMissionSlot = slot;
    if (reset) m_completed = false;
    if (m_completed) return;
    const int type = GetStatType();
    if (reset && m_scope.Equals(k_missionScopeAccumulative)) Statistics::GetInstance()->ResetStatisticsFor(type);
    for (size_t i = 0; i < m_conditions.size(); i++) {
        MissionCondition *c = m_conditions[i];
        c->Reset();
        c->SetStatType(type);
        c->SetRepeatCountValues(repeatCount);
        c->RegisterObserver();
    }
}

// @0x57d338: what the last condition has done, over the number of conditions
float Mission::GetCompletedPercentage() const {
    if (m_completed) return 1.0f;
    float p = 0;
    for (size_t i = 0; i < m_conditions.size(); i++) p = m_conditions[i]->GetCompletedPercentage();
    const float v = p / (float)m_conditions.size();
    return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}

// @0x57d3a8: the missions are for who has the social features, are active, whose dependency was done and whose location is
// open; and whose conditions the game counts
bool Mission::CanBeSeenByUser() {
    if (!Game::GetInstance()->AreSocialFeaturesEnabled() || !m_isActive || !WasMissionDependencyCompleted()) return false;
    if (m_location != -1 && !Player::GetInstance()->IsLocationUnlocked(m_location)) return false;
    bool implemented = true;
    for (size_t i = 0; i < m_conditions.size(); i++) implemented = m_conditions[i]->IsScopeCodeImplemented() && implemented;
    return implemented;
}

// @0x57d460: the type of the statistics it counts: the current run, the career, or one of the accumulated by its slot
int Mission::GetStatType() const {
    if (m_scope.Equals(k_missionScopeCareer)) return 1;
    if (m_scope.Equals(k_missionScopeAccumulative)) return m_activeMissionSlot + 2;
    return 0;
}

// @0x57d4c0
void Mission::ForceConditionUpdate() {
    if (m_completed) return;
    for (size_t i = 0; i < m_conditions.size(); i++) m_conditions[i]->ForceUpdate();
}

// @0x57d500
int Mission::GetFirstConditionNumber() const { return m_conditions.size() > 0 ? m_conditions[0]->GetIntValue() : 0; }

// @0x57d520: the text with the numbers of its conditions in it
const jet::String &Mission::GetDescription() {
    GetParam(jet::String("MissionDescription"), m_description);
    m_description = babel::Babel::GetInstance()->GetStringMgr()->Get(m_description).c_str();
    babel::Formatter *f = babel::Babel::GetInstance()->GetFormatter();
    for (size_t i = 0; i < m_conditions.size(); i++) {
        // GameUtils::Debug_StrFmtValidate is always true in the .so
        if (m_conditions[i]->IsFloatValue())
            m_description = jet::String::Format(m_description.c_str(), f->FormatMeasure(m_conditions[i]->GetFloatValue(), 2).c_str());
        else
            m_description = jet::String::Format(m_description.c_str(), f->FormatMeasure(m_conditions[i]->GetIntValue()).c_str());
    }
    return m_description;
}

// @0x57d6e0: the GUI objects that explain the mission the first time (GuiObject is not ported: nothing is shown, so the
// mission never counts as explained)
void Mission::DisplayGuiAssociated(int) {}

// @0x57d818
void Mission::UnregisterObservers() {
    for (size_t i = 0; i < m_conditions.size(); i++) m_conditions[i]->UnregisterObserver();
}

// @0x57d840
bool Mission::WasMissionDependencyCompleted() const {
    if (!m_hasDependency) return true;
    return MissionMgr::GetInstance()->WasMissionIdCompleted(m_dependencyMissionId);
}
