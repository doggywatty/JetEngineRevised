// source/game/interfaces/menus/Menu_Ingame.cpp
//
// Minimal, deliberately conservative port, same approach as Menu_Shop.cpp: shows the in-game HUD's
// static Aurora page ("Ingame") via Menu_Base's generic BasicPage rendering. Does not call
// GetUIObject() with guessed element names. This is by far the largest menu in this tree; every
// method here is a deliberate no-op stub, not a real implementation -- actual gameplay (scoring,
// perks, vehicles, checkpoints, powerups) needs GS_Gameplay, which doesn't exist yet. See the
// comment at the top of Menu_Ingame.h for what's verified vs guessed about the nested enums.
#include "Menu_Ingame.h"
#include "game/GameLevel.h"
#include "game/Statistics.h"
#include "game/gamestates/GS_Gameplay.h"
#include "sound/soundmgr.h"
#include "game/interfaces/interfaceobjects/InterfaceObject.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"

const jet::String Menu_Ingame::k_menuName("Ingame");
const int Menu_Ingame::k_slotAnimTotalTime = 1000;
const int Menu_Ingame::k_despicableTextTotalAnimTime = 1000;
const int Menu_Ingame::k_despicableTextMoveScaleAnimTime = 500;
const float Menu_Ingame::k_despicableTextTotalFinalZRot = 0.0f;
const int Menu_Ingame::k_powerUpTextShowTime = 1000;
const float Menu_Ingame::k_crossairScaleBase = 1.0f;
const float Menu_Ingame::k_crossairScaleMult = 1.0f;
const float Menu_Ingame::k_crossairMaximumScale = 1.0f;
const float Menu_Ingame::k_crossairMinimumScale = 1.0f;
const float Menu_Ingame::k_crossairMinimumScaleReferenceDistance = 1.0f;
const int Menu_Ingame::k_countdownTime = 3000;
const int Menu_Ingame::k_countdownTimeScaleUp = 200;
const int Menu_Ingame::k_countdownTimeScaleDown = 200;
const float Menu_Ingame::k_score_effectScale = 1.2f;
const int Menu_Ingame::k_score_effectScaleUpTime = 100;
const int Menu_Ingame::k_score_effectScaleBackTime = 100;
const int Menu_Ingame::k_scoreEffectMilestone = 1000;
const int Menu_Ingame::k_scoreEffectDivider1 = 10;
const int Menu_Ingame::k_scoreEffectDivider2 = 100;
const float Menu_Ingame::k_bananas_effectScale = 1.2f;
const int Menu_Ingame::k_bananas_effectScaleUpTime = 100;
const int Menu_Ingame::k_bananas_effectScaleBackTime = 100;
const int Menu_Ingame::k_nextFriendOutTime = 500;
const int Menu_Ingame::k_nextFriendInTime = 500;

Menu_Ingame::Menu_Ingame()
    : Menu_Base(jet::String("Ingame")), m_bossPbOffset(0, 0), m_crossairAlphaDelayer(0.0f, 1.0f, 0), m_despicableProgressBar(0), m_despicableMultiplier(0),
      m_vehiclesClipAndTouchArea(0), m_vehiclesClipAndTouchAreaRect(0, 0, 0, 0), m_despicableProgressBarGlow(0), m_slotAnim(0), m_isChoosingVehicle(false),
      m_oldDespicableMulti(0), m_vehicleChosenIdx(EVehiclesDef::Vehicle_None), m_vehiclePreviousChosenIdx(EVehiclesDef::Vehicle_None), m_slotAnimTime(0),
      m_vehicleChoosingAnimTime(0), m_despicableTextEnabled(false), m_despicableText(0), m_despicableTextStartPos(0, 0), m_despicableTextTargetPos(0, 0),
      m_despicableTextAnimTime(0), m_despicableTextRotationAnim(0), m_despicableTextRotation(0, 0, 0), m_despicableTextOffsetDelayer(0.0f, 1.0f, 0),
      m_powerUpProgressBar(0), m_powerUp3D(0), m_powerUpText(0), m_hasUserBoughtMultiplier(false), m_perkDesc1(0), m_perkDesc2(0), m_countDownTimer(0),
      m_countDownOldValue(0), m_countdownIText(0), m_state(EStateDef::State_Normal), m_preparingToFastForward(false), m_previousScore(0), m_scoreEffectTimer(0),
      m_previousBananas(0), m_bananasEffectTimer(0), m_pause_button(0), m_item1UseButton(0), m_item1ActivatedText(0), m_item2UseButton(0), m_item2ActivatedText(0),
      m_stateBananaMP(EStateMultipliersPerksDef::MP_None), m_stateScoreMP(EStateMultipliersPerksDef::MP_None), m_igNotification_canUpdateOrRender(true),
      m_igNotification_perksRenderOffset(0), m_currentWeekMission(0), m_currentWeekMissionText(0), m_nextFriendToBeat(0), m_nextFriendToBeatAvatarArea(0),
      m_nextFriendToBeatBg(0), m_nextFriendToBeatParent(0), m_nextFriendScore(0), m_nextFriendEffectTimer(0) {}

Menu_Ingame::~Menu_Ingame() {}

const jet::String &Menu_Ingame::GetMenuName() const { return k_menuName; }
bool Menu_Ingame::UpdateGameInBG() { return true; }
bool Menu_Ingame::RenderGameInBG() { return true; }
// @0x4eaa30: the run's counters start from the values already banked (score and bananas minus one, so the first
// change shows), the effect timers are cleared, then the multipliers and the next friend to beat are set up.
void Menu_Ingame::OnEnter() {
    m_scoreEffectTimer = 0;
    m_previousScore = static_cast<int>(Statistics::GetInstance()->GetScore()) - 1;
    m_bananasEffectTimer = 0;
    m_previousBananas = Statistics::GetInstance()->GetCurrentRunBananas() - 1;
    InitStateMultipliers();
    RefreshNextFriendToBeat();
}
void Menu_Ingame::OnQuit() {}

// @0x4e8f28: the page elements the HUD keeps, then the four that are never shown in a run (the next friend's score icon,
// the near-miss text and the two debug texts).
void Menu_Ingame::Init() {
    Menu_Base::Init();
    m_pause_button = GetUIButton(jet::String("pause_button"));
    m_perkDesc1 = GetUIText(jet::String("Item1_Text_description"));
    m_perkDesc2 = GetUIText(jet::String("Item2_Text_description"));
    m_item1UseButton = GetUIButton(jet::String("Item1_UseButton"));
    m_item1ActivatedText = GetUIText(jet::String("Item1_Text_activated"));
    m_item2UseButton = GetUIButton(jet::String("Item2_UseButton"));
    m_item2ActivatedText = GetUIText(jet::String("Item2_Text_activated"));
    m_currentWeekMissionText = GetUIText(jet::String("WeeklyContestObjective_Text"));
    m_nextFriendToBeatBg = GetUIGraph(jet::String("NextFriend_bg"));
    m_nextFriendToBeatAvatarArea = GetUIArea(jet::String("NextFriend_Area"));
    SetElementVisible("NextFriend_Score_icon", false);
    SetElementVisible("NearMissText", false);
    SetElementVisible("currInputRatio_Value", false);
    SetElementVisible("currTime_Value", false);
}

// @0x4e9984: a run starts with none of the run-only elements of the previous state on screen. The despicable meter, the vehicle
// area and the power-up meter and texts are cached and reset; the crossair, the debug buttons, the countdown, the despicable
// texts and the power-up icons are hidden; then the game's perks and checkpoint panels go, the state returns to the normal
// HUD and the loading panel is hidden. The .so's field stores (+0x122, +0x178, +0x17c, +0x180) are mapped to the members
// that hold the same values (the .so's field offsets are not the header's).
void Menu_Ingame::RestartGame() {
    m_isChoosingVehicle = false;
    m_oldDespicableMulti = -1;
    m_slotAnimTime = 0;
    m_vehicleChoosingAnimTime = 0;
    m_despicableProgressBar = GetUIPBar(jet::String("despicable_pbar"));
    if (m_despicableProgressBar) {
        m_despicableProgressBar->SetValue(0);
        m_despicableProgressBar->SetVisible(false);
    }
    m_despicableProgressBarGlow = GetUIGraph(jet::String("despicable_bar_glow"));
    m_vehiclesClipAndTouchArea = GetUIArea(jet::String("VehicleClipAndTouchArea"));
    m_slotAnim = GetUIGraph(jet::String("despicable_slotAnim"));
    m_powerUpProgressBar = GetUIPBar(jet::String("powerup_pbar"));
    if (m_powerUpProgressBar) {
        m_powerUpProgressBar->SetValue(0);
        m_powerUpProgressBar->SetVisible(false);
    }
    m_powerUpText = GetUIText(jet::String("powerup_text"));
    m_countdownIText = GetUIText(jet::String("CountDown"));
    m_despicableText = GetUIText(jet::String("Despicable_Text"));
    SetElementVisible("despicable_bar_glow", false);
    SetElementVisible("despicable_slotAnim", false);
    SetElementVisible("crossair", false);
    SetElementVisible("FluffyDebugButton", false);
    SetElementVisible("FreezeGunDebugButton", false);
    SetElementVisible("CostumeDebugLast", false);
    SetElementVisible("CostumeDebugNext", false);
    SetElementVisible("GiantDebugButton", false);
    SetElementVisible("CountDown", false);
    SetElementVisible("Despicable_Text", false);
    SetElementVisible("Despicable_TargetPos", false);
    SetElementVisible("powerup_text", false);
    SetElementVisible("powerup_magnet", false);
    SetElementVisible("powerup_shield", false);
    SetElementVisible("powerup_bananas", false);
    SetElementVisible("powerup_freezegun", false);
    if (GameLevel *level = GameLevel::GetInstance()) {
        level->ShowPerksUI(false);
        level->ShowCheckpointUI(false);
    }
    SetState(EStateDef::State_Normal);
    SetLoadingPanelVisible(false);
}

// Hides or shows one element of the page by name (GetUIObject(name)->SetVisible); an element the page does not have is skipped.
void Menu_Ingame::SetElementVisible(const char *name, bool visible) {
    if (InterfaceObject *o = GetUIObject(jet::String(name))) o->SetVisible(visible);
}

// @0x4efce8
void Menu_Ingame::SetLoadingPanelVisible(bool visible) { SetElementVisible("LoadingPanel", visible); }
void Menu_Ingame::Update(int dt) {
    Menu_Base::Update(dt);
    UpdateCountdown(dt);
}
void Menu_Ingame::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { Menu_Base::Render(cam, p); }
void Menu_Ingame::SetDespicablePointsPgt(float) {}
void Menu_Ingame::EnableDespicableBlinking(bool) {}
void Menu_Ingame::StartDespicableAnim(math::vec3<float>) {}
void Menu_Ingame::UpdateDespicableAnim(int) {}
void Menu_Ingame::SetBossPgt(float) {}
void Menu_Ingame::SetFluffyMessageEnabled(bool, const char*) {}
bool Menu_Ingame::IsChoosingVehicle() { return m_isChoosingVehicle; }
void Menu_Ingame::EnableDebugSlider1(bool) {}
bool Menu_Ingame::IsDebugSlider1Enabled() { return false; }
float Menu_Ingame::GetDebugSlider1Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider1Value(float) {}
void Menu_Ingame::SetDebugSlider1Text(const char*) {}
void Menu_Ingame::EnableDebugSlider2(bool) {}
bool Menu_Ingame::IsDebugSlider2Enabled() { return false; }
float Menu_Ingame::GetDebugSlider2Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider2Value(float) {}
void Menu_Ingame::SetDebugSlider2Text(const char*) {}
void Menu_Ingame::EnableDebugSlider3(bool) {}
bool Menu_Ingame::IsDebugSlider3Enabled() { return false; }
float Menu_Ingame::GetDebugSlider3Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider3Value(float) {}
void Menu_Ingame::SetDebugSlider3Text(const char*) {}
void Menu_Ingame::EnableDebugSlider4(bool) {}
bool Menu_Ingame::IsDebugSlider4Enabled() { return false; }
float Menu_Ingame::GetDebugSlider4Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider4Value(float) {}
void Menu_Ingame::SetDebugSlider4Text(const char*) {}
void Menu_Ingame::EnableDebugSlider5(bool) {}
bool Menu_Ingame::IsDebugSlider5Enabled() { return false; }
float Menu_Ingame::GetDebugSlider5Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider5Value(float) {}
void Menu_Ingame::SetDebugSlider5Text(const char*) {}
void Menu_Ingame::EnableDebugSlider6(bool) {}
bool Menu_Ingame::IsDebugSlider6Enabled() { return false; }
float Menu_Ingame::GetDebugSlider6Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider6Value(float) {}
void Menu_Ingame::SetDebugSlider6Text(const char*) {}
void Menu_Ingame::EnableDebugSlider7(bool) {}
bool Menu_Ingame::IsDebugSlider7Enabled() { return false; }
float Menu_Ingame::GetDebugSlider7Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider7Value(float) {}
void Menu_Ingame::SetDebugSlider7Text(const char*) {}
void Menu_Ingame::EnableDebugSlider8(bool) {}
bool Menu_Ingame::IsDebugSlider8Enabled() { return false; }
float Menu_Ingame::GetDebugSlider8Value() { return 0.0f; }
void Menu_Ingame::SetDebugSlider8Value(float) {}
void Menu_Ingame::SetDebugSlider8Text(const char*) {}
// @0x4ede84: the resume countdown starts at 3000 ms, its text is shown, and the old second is forgotten
void Menu_Ingame::StartCountdown() {
    m_countDownOldValue = -1;
    m_countDownTimer = 3000;
    if (m_countdownIText) m_countdownIText->SetVisible(true);
}
bool Menu_Ingame::IsCountdownEnabled() { return m_countDownTimer > 0; }
void Menu_Ingame::InitDebugSliders() {}
bool Menu_Ingame::IsDebugSliderEnabled(const jet::String&) { return false; }
void Menu_Ingame::EnableDebugSlider(const jet::String&, const jet::String&, bool) {}
void Menu_Ingame::SetDebugSliderText(const jet::String&, const char*) {}
void Menu_Ingame::SetDebugSliderText() {}
float Menu_Ingame::GetDebugSliderValue(const jet::String&) { return 0.0f; }
void Menu_Ingame::SetDebugSliderValue(const jet::String&, float) {}
void Menu_Ingame::UpdateStateNormal() {}
void Menu_Ingame::UpdateStateCheckpoint() {}
void Menu_Ingame::UpdateStatePerks() {}
void Menu_Ingame::UpdatePowerUps() {}
void Menu_Ingame::_UpdateBananaPerkData() {}
void Menu_Ingame::_UseBananaPerk() {}
void Menu_Ingame::_UpdateDespicableMultiplierData() {}
void Menu_Ingame::_UseDespicableMultiplier() {}
// @0x4edeb0: the countdown runs down by the frame's time (ms). The touches follow the countdown (they are off while it runs). Each
// second shows its number (the countdown's number plus one) and plays the countdown sound once; the number pulses over the second (it grows in
// the first 200 ms and shrinks in the last 200 ms, scale 1 in between). When it is over the text is hidden and the game resumes.
void Menu_Ingame::UpdateCountdown(int dt) {
    const bool running = m_countDownTimer > 0;
    if (m_touchesEnabled != running) EnableTouches(!running);
    if (!running) return;
    m_countDownTimer -= dt;
    if (m_countDownTimer < 0) {
        m_countDownTimer = 0;
        m_countDownOldValue = -1;
        if (m_countdownIText) m_countdownIText->SetVisible(false);
        if (GS_Gameplay *gameplay = GS_Gameplay::GetInstance()) gameplay->OnResumeCountdownEnded();
        return;
    }
    const int second = m_countDownTimer / 1000;
    if (m_countdownIText) {
        m_countdownIText->SetText(jet::String::Format("%d", second + 1));
        if (m_countDownOldValue != second) {
            m_countDownOldValue = second;
            SoundMgr::GetInstance()->Play(jet::String("ev_sfx_countdown"), 0);
        }
        const int ms = m_countDownTimer - 1000 * second;
        float scale = 1.0f;
        if (ms > 800) scale = (1000 - ms) * 0.005f;
        else if (ms <= 199) scale = ms * 0.005f;
        m_countdownIText->SetScale(math::vec2<float>(scale, scale));
    }
}

// @0x4ef26c: the touches of the HUD (the stored flag, and the buttons and texts of the HUD follow it)
void Menu_Ingame::EnableTouches(bool on) {
    m_touchesEnabled = on;
    if (m_pause_button) m_pause_button->SetActive(on);
    if (m_item1UseButton) m_item1UseButton->SetActive(on);
    if (m_item2UseButton) m_item2UseButton->SetActive(on);
    if (m_perkDesc1) m_perkDesc1->SetActive(on);
    if (m_perkDesc2) m_perkDesc2->SetActive(on);
    if (m_powerUpText) m_powerUpText->SetActive(on);
}
// @0x4ee060: the three panels of the HUD follow the state: the normal HUD for 0, the checkpoint panel for 1, the perks panel for 2.
// The .so's perks panel placement that follows (IGNotificationMgr::GetRenderedHeight and the banana/multiplier perk refresh)
// is not ported yet, see re/TODO.md.
void Menu_Ingame::SetState(safe_enum<Menu_Ingame::EStateDef, Menu_Ingame::EStateDef::type> s) {
    m_state = s;
    const EStateDef::type st = s;
    if (InterfaceObject *normal = GetUIObject(jet::String("State_NormalUI"))) normal->SetVisible(st == EStateDef::State_Normal);
    if (InterfaceObject *perks = GetUIObject(jet::String("State_Perks"))) perks->SetVisible(st == EStateDef::State_Perks);
    if (InterfaceObject *checkpoint = GetUIObject(jet::String("State_Checkpoint"))) checkpoint->SetVisible(st == EStateDef::State_Checkpoint);
}
void Menu_Ingame::_UpdateCheckpointData() {}
void Menu_Ingame::UpdateScoreTextAndEffect(int) {}
void Menu_Ingame::UpdateBananaTextAndEffect(int) {}
void Menu_Ingame::ResetScaleEffectTimer(int&, const int, const int) {}
void Menu_Ingame::UpdateScaleAndEffectTimer(int, float&, int&, const int, const int) {}
void Menu_Ingame::UpdatePauseButtonPress() {}
void Menu_Ingame::InitStateMultipliers() {}
void Menu_Ingame::UpdateLogicPerks() {}
void Menu_Ingame::UpdateBananaLogicPerks() {}
void Menu_Ingame::UpdateScoreLogicPerks() {}
void Menu_Ingame::UpdateNextFriendToBeat(int) {}
void Menu_Ingame::RefreshNextFriendToBeat() {}
TRACKING_SCREEN Menu_Ingame::GetCurrentTrackingScreen() { return TRACKING_SCREEN(0); }
