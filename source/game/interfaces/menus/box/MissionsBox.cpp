// source/game/interfaces/menus/box/MissionsBox.cpp
//
// Not ported: GameTrackingMgr::SendCurrencySpentEvent (the skip), Mission::DisplayGuiAssociated
// (the GUI object that explains a mission; GuiObject is not ported).
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "MissionsBox.h"
#include "game/Player.h"
#include "game/entities/Mission.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "project.h"
#include "system.h"
#include "video/display.h"
#include <cmath>
#include <functional>

math::vec2<float> MissionsBox::k_introOffset(0, 0);   // @0xd0d018
math::vec2<float> MissionsBox::k_outroOffset(0, 0);   // @0xd0d020

// @0x4d54a4: the box of the mission of the name; a mission of a scope that counts over several runs is brought up to date
// first
MissionsBox::MissionsBox(const jet::String &missionName, bool isNew, int slot)
    : BasicPage(jet::String("MissionsBox")), m_missionName(missionName), m_isNew(isNew), m_completed(false), m_missionPoints(0), m_enableExitScreenAnim(true),
      m_completeButton(0), m_state(0), m_introDelayer(math::vec2<float>(0, 0), math::vec2<float>(0, 0), 1000),
      m_outroDelayer(math::vec2<float>(0, 0), math::vec2<float>(0, 0), 1000), m_completedTimer(0), m_slot(slot), m_boxOriginalOffset(0, 0), m_outroDistance(0),
      m_mission(0), m_missionGiverPBar(0) {
    InterfaceObject::SetSize(GetUIObject(jet::String("BoxSize"))->GetSize());
    m_mission = static_cast<Mission*>(clara::Project::GetInstance()->FindEntityByName(m_missionName));
    if (!m_mission->IsCompleted() && !m_mission->GetScope().Equals(Mission::k_missionScopeCurrentRun)) m_mission->ForceConditionUpdate();
    m_completed = m_mission->IsCompleted();
    m_missionPoints = (int)m_mission->GetMissionMultiplierPoints();
    InterfaceText *text = GetUIText(jet::String("mission_txt"));
    // "MISSIONS" and the number of the slot in orange, then the mission's own text
    const jet::String number = jet::String::Format("#FF6d02%s%d:#396070 ", babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("MISSIONS")).c_str(), m_slot + 1);
    text->SetText(number + m_mission->GetDescription());
    GetUIObject(jet::String("mission_finished"))->SetVisible(m_completed);
    // TutorialMgr::IsTutorialFinished(k_firstPlay) is asked and not used by the .so
    m_completeButton = GetUIButton(jet::String("mission_completeButton"));
    m_completeButton->SetVisible(false);
    m_completeButton->SetText(jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("SKIP_MISSION")).c_str(),
                                                  (int)m_mission->GetSkipPrice()->GetPriceAsDouble()));
    InitMissionGiverPBar();
    m_state = m_isNew ? State_WaitIntro : State_Shown;
    m_completedTimer = m_completed ? 2000 : 0;
    MissionMgr::GetInstance()->MarkIGNotifyShownFor(m_mission->GetName());
    if (std::fabs(k_introOffset.x) <= 1.1920929e-07f && std::fabs(k_introOffset.y) <= 1.1920929e-07f) {  // math::isZero
        // the first box: the new missions come from beyond the right side of the screen, the done ones go beyond the left
        const float width = (float)jet::System::GetDisplay(0)->GetSize().x;
        k_introOffset = math::vec2<float>(width + width, 0);
        k_outroOffset = math::vec2<float>(width * -2.0f, 0);
    }
}

// @0x4d5a3c
MissionsBox::~MissionsBox() {}

// @0x4d5a94: the intro, the wait of a mission that is done, the outro; a release of the skip button buys the skip
void MissionsBox::Update(int dt) {
    BasicPage::Update(dt);
    MissionMgr::GetInstance()->MarkIGNotifyShownFor(m_mission->GetName());
    if (m_state == State_Shown) {
        if (m_completed && m_enableExitScreenAnim) {
            m_completedTimer -= dt;
            if (m_completedTimer <= 0) {
                m_completedTimer = 0;
                m_state = State_Finished;
                TryStartIntroOutroAnim();
            }
        }
        m_mission->DisplayGuiAssociated(dt);
    } else if (m_state == State_Outro) {
        if (m_outroDelayer.IsUpdating()) {
            float remaining = m_outroDelayer.GetCurrentValue().y - m_outroDelayer.GetTargetValue().y;
            if (remaining > 90.0f) {
                // far from the end: it moves with its full speed, and shrinks as it goes
                remaining /= m_outroDistance;
                m_outroDelayer.Update(dt);
                InterfaceObject::SetOffset(m_outroDelayer.GetCurrentValue());
                SetUIScale(1.0f + (1.0f - remaining) * 0.3f);
            } else {
                // near the end: at half the speed
                m_outroDelayer.Update(dt / 2);
                remaining *= 0.011111111f;
                SetUIScale(0.5f + remaining * (0.5f + (1.0f - 90.0f / m_outroDistance) * 0.3f));
            }
        }
        if (!m_outroDelayer.IsUpdating()) {
            m_mission->DisplayGuiAssociated(dt);
            m_state = State_Done;
        }
    } else if (m_state == State_Intro) {
        if (m_introDelayer.IsUpdating()) {
            m_introDelayer.Update(dt);
            InterfaceObject::SetOffset(m_introDelayer.GetCurrentValue());
        }
        if (!m_introDelayer.IsUpdating()) m_state = State_Shown;
    }
    MenuMgr *mm = MenuMgr::GetInstance();
    const int count = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < count; i++)
        if (m_completeButton == mm->ACTION_GetReleasedButton(i)) {
            BuySkipMission(0, 0);
            return;
        }
}

// @0x4d5ca8
void MissionsBox::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (!IsActive()) return;
    BasicPage::Render(cam, p);
}

// @0x4d5ce0: where the box rests; a new one that has not come in yet waits beyond the side
void MissionsBox::SetOffset(const math::vec2<float> &offset) {
    m_boxOriginalOffset = offset;
    if (m_isNew && m_state == State_WaitIntro)
        InterfaceObject::SetOffset(math::vec2<float>(k_introOffset.x + offset.x, k_introOffset.y + offset.y));
    else
        InterfaceObject::SetOffset(offset);
}

// @0x4d5d58: where the done missions go to: a little to the right and 110 above where the multiplier bar is
void MissionsBox::SetOutroPos(const math::vec2<float> &pos) { k_outroOffset = math::vec2<float>(pos.x + 10.0f, pos.y - 110.0f); }

// @0x4d5d98
bool MissionsBox::HasIntroOrOutroAnim() const {
    if (m_state == State_Shown && !m_completed) return false;
    return m_state != State_Done;
}

// @0x4d5dbc: a new mission comes in; a done one goes out and its text is the slot's only
void MissionsBox::TryStartIntroOutroAnim() {
    if (m_state == State_WaitIntro) {
        m_state = State_Intro;
        m_introDelayer.SetCurrentValue(InterfaceObject::GetOffsetNoParent());
        m_introDelayer.SetTargetValue(m_boxOriginalOffset);
        m_introDelayer.SetDelay(500);
    }
    if (m_state == State_Finished) {
        m_state = State_Outro;
        m_outroDelayer.SetCurrentValue(GetOffsetNoParent());
        m_outroDelayer.SetTargetValue(k_outroOffset);
        m_outroDelayer.SetDelay(700);
        m_outroDistance = m_outroDelayer.GetCurrentValue().y - m_outroDelayer.GetTargetValue().y;
        GetUIText(jet::String("mission_txt"))
            ->SetText(jet::String::Format("#FF6d02%s%d#396070 ", babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("MISSIONS")).c_str(), m_slot + 1));
    }
}

// @0x4d5f18
bool MissionsBox::AnyDelayersOn() const { return m_completedTimer > 0 || m_outroDelayer.IsUpdating() || m_introDelayer.IsUpdating(); }

// @0x4d5f40: only the bar of the giver of the mission is shown, at the percentage the mission is at, moving to it
void MissionsBox::InitMissionGiverPBar() {
    GetUIPBar(jet::String("missions_NefarioPBar"))->SetVisible(false);
    GetUIPBar(jet::String("missions_GruPBar"))->SetVisible(false);
    GetUIPBar(jet::String("missions_GirlsPBar"))->SetVisible(false);
    m_missionGiverPBar = 0;
    if (m_mission->GetMissionGiver().Equals(Mission::k_missionGiver_Girls)) m_missionGiverPBar = GetUIPBar(jet::String("missions_GirlsPBar"));
    if (m_mission->GetMissionGiver().Equals(Mission::k_missionGiver_Gru)) m_missionGiverPBar = GetUIPBar(jet::String("missions_GruPBar"));
    if (m_mission->GetMissionGiver().Equals(Mission::k_missionGiver_Nefario)) m_missionGiverPBar = GetUIPBar(jet::String("missions_NefarioPBar"));
    m_missionGiverPBar->SetVisible(true);
    m_missionGiverPBar->SetValue(0.0f);
    m_missionGiverPBar->Update(1);
    m_missionGiverPBar->EnableInertia(true, InterfacePBar::k_defaultInertiaMultiPerMillisecond);
    m_missionGiverPBar->SetValue(m_mission->GetCompletedPercentage());
}

// @0x4d60c0: the mission is skipped for what its price is (tokens); it is then done
void MissionsBox::BuySkipMission(int, int) {
    if (m_completed) return;
    if (!Player::GetInstance()->ExecuteCost(*m_mission->GetSkipPrice(), [this](int a, int b) { BuySkipMission(a, b); })) return;
    SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_buy, 0);
    m_mission->OnForceCompletion();
    m_missionGiverPBar->SetValue(m_mission->GetCompletedPercentage());
    m_completed = true;
    m_completedTimer = 2000;
    GetUIObject(jet::String("mission_finished"))->SetVisible(m_completed);
    // GameTrackingMgr::SendCurrencySpentEvent(tokens, mission item, the giver, the first condition's number, the price)
}

// @0x4d62d4: a mission that counts over several runs is looked at again, and may be done by now
void MissionsBox::TryForcingConditionUpdate() {
    if (m_state == State_Done || m_completed) return;
    if (m_mission->GetScope().Equals(Mission::k_missionScopeCurrentRun)) return;
    m_mission->ForceConditionUpdate();
    m_completed = m_mission->IsCompleted();
    m_missionGiverPBar->SetValue(m_mission->GetCompletedPercentage());
    GetUIObject(jet::String("mission_finished"))->SetVisible(m_completed);
    m_completedTimer = m_completed ? 2000 : 0;
}
