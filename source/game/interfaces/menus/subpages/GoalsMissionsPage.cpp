// source/game/interfaces/menus/subpages/GoalsMissionsPage.cpp
#include "GoalsMissionsPage.h"
#include "game/Player.h"
#include "game/gamemanagers/BappleMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceArea.h"
#include "game/interfaces/menus/box/MissionsBox.h"
#include "game/interfaces/menus/box/MissionsMultiplierBox.h"

// @0x4a0110: the size of the page is the one of "mission_size"; the state marks of the other tabs' layers are hidden
GoalsMissionsPage::GoalsMissionsPage() : BasicPage(jet::String("Menu_Missions")), m_missionsMultiplierBox(0), m_missionsParentObj(0) {
    InterfaceObject::SetSize(GetUIObject(jet::String("mission_size"))->GetSize());
    GetUIObject(jet::String("Achievements_State"))->SetVisible(false);
}

// @0x4a02c8
GoalsMissionsPage::~GoalsMissionsPage() {
    _ClearMissionBoxes();
    delete m_missionsMultiplierBox;
}

// where the done missions go: the .so takes the middle of the multiplier bar with the height of its size in both axes
math::vec2<float> GoalsMissionsPage::GetOutroPos() {
    const math::vec2<float> &barOffset = m_missionsMultiplierBox->GetOffsetNoParent();
    const math::vec2<float> barSize = m_missionsMultiplierBox->GetSize();
    return math::vec2<float>(barOffset.x + barSize.y * 0.5f, barOffset.y + barSize.y * 0.5f);
}

// @0x4a0360: a mission that went out is taken from its slot: its points go to the multiplier bar, and a new mission, if
// the slot has one, comes in under the others; then what the player has done unlocks (locations and the bapples hunt)
void GoalsMissionsPage::Update(int dt) {
    BasicPage::Update(dt);
    if (!m_missionsParentObj) return;  // (not refreshed yet: the .so is never updated then)
    const math::vec2<float> parentPos = m_missionsParentObj->GetPos();
    for (size_t i = 0; i < m_missionsBox.size(); i++) {
        MissionsBox *box = m_missionsBox[i];
        if (box->GetState() != MissionsBox::State_Done) continue;
        m_missionsMultiplierBox->AddMissionMultiplierPoints(box->GetMissionPoints());
        MissionMgr *mm = MissionMgr::GetInstance();
        mm->RemoveCompletedMission((int)i);
        mm->ComputeGameMissions();
        const MissionMgrSaveData::ActiveMissionInfo &info = mm->GetActiveMissionInfo((unsigned)i);
        if (info.state == 0) continue;
        MissionsBox *fresh = new MissionsBox(info.id, true, (int)i);
        fresh->SetParent(m_missionsParentObj);
        // the new box rests where the slot is: the height of the boxes before it
        const math::vec2<float> boxSize = fresh->GetSize();
        const math::vec2<float> parentSize = m_missionsParentObj->GetSize();
        const float step = boxSize.y + parentSize.y;
        fresh->SetOffset(math::vec2<float>(parentPos.x, parentPos.y + (float)i * step));
        // the done missions go out towards the middle of the multiplier bar
        fresh->SetOutroPos(GetOutroPos());
        fresh->TryStartIntroOutroAnim();
        delete m_missionsBox[i];
        m_missionsBox[i] = fresh;
    }
    Player::GetInstance()->ComputeLocationUnlock();
    BappleMgr::GetInstance()->ComputeBappleHuntUnlock();
}

// @0x4a079c
void GoalsMissionsPage::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    BasicPage::Render(cam, p);
    for (size_t i = 0; i < m_missionsBox.size(); i++) m_missionsBox[i]->Render(cam, p);
}

// @0x4a07e8
void GoalsMissionsPage::_ClearMissionBoxes() {
    for (size_t i = 0; i < m_missionsBox.size(); i++) delete m_missionsBox[i];
    m_missionsBox.clear();
}

// @0x4a0574: the boxes of the active missions again, one under the other, and the multiplier bar
void GoalsMissionsPage::Refresh() {
    _ClearMissionBoxes();
    InitMissionMultiplier();
    m_missionsParentObj = GetUIArea(jet::String("mission_missionsOffset"));
    math::vec2<float> offset = m_missionsParentObj->GetPos();
    MissionMgr *mm = MissionMgr::GetInstance();
    for (unsigned i = 0; i < mm->GetActiveMissionCount(); i++) {
        const MissionMgrSaveData::ActiveMissionInfo &info = mm->GetActiveMissionInfo(i);
        if (info.state == 0) continue;
        MissionsBox *box = new MissionsBox(info.id, false, (int)i);
        box->SetParent(m_missionsParentObj);
        box->SetOffset(offset);
        box->SetOutroPos(GetOutroPos());
        m_missionsBox.push_back(box);
        const math::vec2<float> boxSize = box->GetSize();
        const math::vec2<float> parentSize = m_missionsParentObj->GetSize();
        offset.y = offset.y + (boxSize.y + parentSize.y);
    }
}

// @0x4a0830: a new bar of the multiplier, under the missions
void GoalsMissionsPage::InitMissionMultiplier() {
    delete m_missionsMultiplierBox;
    m_missionsMultiplierBox = new MissionsMultiplierBox(false);
    InterfaceArea *anchor = GetUIArea(jet::String("mission_nextMultiplierOffset"));
    m_missionsMultiplierBox->SetOffset(anchor->GetPos());
    m_missionsMultiplierBox->SetParent(anchor);
}
