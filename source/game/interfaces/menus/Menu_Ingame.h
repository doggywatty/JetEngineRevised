// source/game/interfaces/menus/Menu_Ingame.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_INGAME_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_INGAME_H

// See Menu_Shop.h for why these additions were needed (no #includes at all originally, GetMenuName()
// missing `const`). This is the real in-game HUD (score/bananas/despicable meter/powerups/pause
// button/vehicle choice/next-friend-to-beat) -- by far the largest menu header in this tree (60+
// methods). Only reachable in practice once GS_Gameplay exists (still a 1-line stub as of this
// session), so this is render-plumbing-only for now: the Aurora page shows with placeholder/zero
// stats, none of the actual gameplay logic (scoring, perks, vehicle switching, checkpoints) is
// wired. Treat every method body in the .cpp as a deliberate no-op stub, not a real implementation.
#include "Menu_Base.h"
#include "jenum.h"
#include "core/delayer.h"
#include "core/interputils.h"
#include "game/online/tracking/tracking.h"
#include <boost/shared_ptr.hpp>
class ParticleSystem2D;
namespace ps { class ParticleSystem; }
class Interface3DElement;
class WeeklyMission;
class OnlineUser;

// EVehiclesDef is referenced bare (not nested) by both this header and source/game/cheats/CheatsMgr.h
// (also never declared there) -- a real global type, not Menu_Ingame-specific. Declared here since
// this is the first place in the tree that actually needs it to compile; move it to a shared header
// if CheatsMgr.cpp is ever ported too. Value count NOT recovered from disassembly (not critical path
// for a render-only pass) -- placeholder only.
struct EVehiclesDef { enum type { Vehicle_None = 0, Vehicle_1 = 1, Vehicle_2 = 2 }; };

class Menu_Ingame : public Menu_Base {  // sizeof 0x2AC
public:
    // Not in the PDB header. Menu_Ingame::SetState() at 0x4ee060 branches on cmp #1/#2 against the
    // new state value, and the method names below it (UpdateStateNormal/UpdateStateCheckpoint/
    // UpdateStatePerks) line up with a 3-value {Normal=0, Checkpoint=1, Perks=2} reading -- plausible
    // from both the disassembly shape and the naming, but the exact member names are not
    // independently confirmed (unlike e.g. Menu_Pause::ExitOptions, where the call targets at each
    // branch were unambiguous).
    struct EStateDef { enum type { State_Normal = 0, State_Checkpoint = 1, State_Perks = 2 }; };
    // Not recovered from disassembly (not critical path for a render-only pass); used only by the
    // banana/score multiplier-perk bookkeeping, which this session's minimal .cpp doesn't implement.
    struct EStateMultipliersPerksDef { enum type { MP_None = 0, MP_Active = 1 }; };
    virtual const jet::String &GetMenuName() const;
    Menu_Ingame();
    virtual ~Menu_Ingame();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Init();
    void RestartGame();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetDespicablePointsPgt(float);
    void EnableDespicableBlinking(bool);
    void StartDespicableAnim(math::vec3<float>);
    void UpdateDespicableAnim(int);
    void SetBossPgt(float);
    void SetFluffyMessageEnabled(bool, const char*);
    bool IsChoosingVehicle();
    void EnableDebugSlider1(bool);
    bool IsDebugSlider1Enabled();
    float GetDebugSlider1Value();
    void SetDebugSlider1Value(float);
    void SetDebugSlider1Text(const char*);
    void EnableDebugSlider2(bool);
    bool IsDebugSlider2Enabled();
    float GetDebugSlider2Value();
    void SetDebugSlider2Value(float);
    void SetDebugSlider2Text(const char*);
    void EnableDebugSlider3(bool);
    bool IsDebugSlider3Enabled();
    float GetDebugSlider3Value();
    void SetDebugSlider3Value(float);
    void SetDebugSlider3Text(const char*);
    void EnableDebugSlider4(bool);
    bool IsDebugSlider4Enabled();
    float GetDebugSlider4Value();
    void SetDebugSlider4Value(float);
    void SetDebugSlider4Text(const char*);
    void EnableDebugSlider5(bool);
    bool IsDebugSlider5Enabled();
    float GetDebugSlider5Value();
    void SetDebugSlider5Value(float);
    void SetDebugSlider5Text(const char*);
    void EnableDebugSlider6(bool);
    bool IsDebugSlider6Enabled();
    float GetDebugSlider6Value();
    void SetDebugSlider6Value(float);
    void SetDebugSlider6Text(const char*);
    void EnableDebugSlider7(bool);
    bool IsDebugSlider7Enabled();
    float GetDebugSlider7Value();
    void SetDebugSlider7Value(float);
    void SetDebugSlider7Text(const char*);
    void EnableDebugSlider8(bool);
    bool IsDebugSlider8Enabled();
    float GetDebugSlider8Value();
    void SetDebugSlider8Value(float);
    void SetDebugSlider8Text(const char*);
    void StartCountdown();
    bool IsCountdownEnabled();
private:
    void InitDebugSliders();
    bool IsDebugSliderEnabled(const jet::String&);
    void EnableDebugSlider(const jet::String&, const jet::String&, bool);
    void SetDebugSliderText(const jet::String&, const char*);
    void SetDebugSliderText();
    float GetDebugSliderValue(const jet::String&);
    void SetDebugSliderValue(const jet::String&, float);
    void SetElementVisible(const char*, bool);
    void SetLoadingPanelVisible(bool);
    void UpdateStateNormal();
    void UpdateStateCheckpoint();
    void UpdateStatePerks();
    void UpdatePowerUps();
    void _UpdateBananaPerkData();
    void _UseBananaPerk();
    void _UpdateDespicableMultiplierData();
    void _UseDespicableMultiplier();
    void UpdateCountdown(int);
    // @0x4ef26c: the HUD touches (the flag at the .so's 0x28C)
    void EnableTouches(bool);
    void SetState(safe_enum<Menu_Ingame::EStateDef,enum Menu_Ingame::EStateDef::type>);
    void _UpdateCheckpointData();
    void UpdateScoreTextAndEffect(int);
    void UpdateBananaTextAndEffect(int);
    void ResetScaleEffectTimer(int&, const int, const int);
    void UpdateScaleAndEffectTimer(int, float&, int&, const int, const int);
    void UpdatePauseButtonPress();
    void InitStateMultipliers();
    void UpdateLogicPerks();
    void UpdateBananaLogicPerks();
    void UpdateScoreLogicPerks();
    void UpdateNextFriendToBeat(int);
    void RefreshNextFriendToBeat();
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
public:
    static const jet::String k_menuName;
private:
    jet::String m_debugSlideText1;  // 0x124
    jet::String m_debugSlideText2;  // 0x128
    jet::String m_debugSlideText3;  // 0x12C
    jet::String m_debugSlideText4;  // 0x130
    jet::String m_debugSlideText5;  // 0x134
    jet::String m_debugSlideText6;  // 0x138
    jet::String m_debugSlideText7;  // 0x13C
    jet::String m_debugSlideText8;  // 0x140
    math::vec2<float> m_bossPbOffset;  // 0x144
    jet::core::Delayer<float,jet::core::SmootherStep> m_crossairAlphaDelayer;  // 0x14C
    InterfacePBar *m_despicableProgressBar;  // 0x164
    InterfaceText *m_despicableMultiplier;  // 0x168
    InterfaceArea *m_vehiclesClipAndTouchArea;  // 0x16C
    math::rect<float> m_vehiclesClipAndTouchAreaRect;  // 0x170
    InterfaceGraph *m_despicableProgressBarGlow;  // 0x180
    InterfaceGraph *m_slotAnim;  // 0x184
    std::vector<InterfaceGraph *,std::allocator<InterfaceGraph *> > m_vehiclesAnim;  // 0x188
    bool m_isChoosingVehicle;  // 0x19C
    int m_oldDespicableMulti;  // 0x1A0
    safe_enum<EVehiclesDef,enum EVehiclesDef::type> m_vehicleChosenIdx;  // 0x1A4
    safe_enum<EVehiclesDef,enum EVehiclesDef::type> m_vehiclePreviousChosenIdx;  // 0x1A8
    int m_slotAnimTime;  // 0x1AC
    int m_vehicleChoosingAnimTime;  // 0x1B0
    static const int k_slotAnimTotalTime;
    bool m_despicableTextEnabled;  // 0x1B4
    InterfaceText *m_despicableText;  // 0x1B8
    math::vec2<float> m_despicableTextStartPos;  // 0x1BC
    math::vec2<float> m_despicableTextTargetPos;  // 0x1C4
    int m_despicableTextAnimTime;  // 0x1CC
    float m_despicableTextRotationAnim;  // 0x1D0
    math::vec3<float> m_despicableTextRotation;  // 0x1D4
    static const int k_despicableTextTotalAnimTime;
    static const int k_despicableTextMoveScaleAnimTime;
    static const float k_despicableTextTotalFinalZRot;
    jet::core::Delayer<float,jet::core::QuarticEaseInOut> m_despicableTextOffsetDelayer;  // 0x1E0
    boost::shared_ptr<ParticleSystem2D> m_particleWrapperDespLabel;  // 0x1F8
    boost::shared_ptr<ps::ParticleSystem> m_particleSystemDespLabel;  // 0x200
    boost::shared_ptr<ParticleSystem2D> m_particleWrapperDespBar;  // 0x208
    boost::shared_ptr<ps::ParticleSystem> m_particleSystemDespBar;  // 0x210
    InterfacePBar *m_powerUpProgressBar;  // 0x218
    std::vector<InterfaceGraph *,std::allocator<InterfaceGraph *> > m_powerUpAnims;  // 0x21C
    Interface3DElement *m_powerUp3D;  // 0x230
    InterfaceText *m_powerUpText;  // 0x234
    static const int k_powerUpTextShowTime;
    static const float k_crossairScaleBase;
    static const float k_crossairScaleMult;
    static const float k_crossairMaximumScale;
    static const float k_crossairMinimumScale;
    static const float k_crossairMinimumScaleReferenceDistance;
    bool m_hasUserBoughtMultiplier;  // 0x238
    InterfaceText *m_perkDesc1;  // 0x23C
    InterfaceText *m_perkDesc2;  // 0x240
    static const int k_countdownTime;
    static const int k_countdownTimeScaleUp;
    static const int k_countdownTimeScaleDown;
    int m_countDownTimer;  // 0x244
    int m_countDownOldValue;  // 0x248
    bool m_touchesEnabled = false;  // the .so's byte at 0x28C (set by EnableTouches)
    InterfaceText *m_countdownIText;  // 0x24C
    safe_enum<Menu_Ingame::EStateDef,enum Menu_Ingame::EStateDef::type> m_state;  // 0x250
    bool m_preparingToFastForward;  // 0x254
    static const float k_score_effectScale;
    static const int k_score_effectScaleUpTime;
    static const int k_score_effectScaleBackTime;
    static const int k_scoreEffectMilestone;
    static const int k_scoreEffectDivider1;
    static const int k_scoreEffectDivider2;
    int m_previousScore;  // 0x258
    int m_scoreEffectTimer;  // 0x25C
    static const float k_bananas_effectScale;
    static const int k_bananas_effectScaleUpTime;
    static const int k_bananas_effectScaleBackTime;
    int m_previousBananas;  // 0x260
    int m_bananasEffectTimer;  // 0x264
    InterfaceButton *m_pause_button;  // 0x268
    InterfaceButton *m_item1UseButton;  // 0x26C
    InterfaceText *m_item1ActivatedText;  // 0x270
    InterfaceButton *m_item2UseButton;  // 0x274
    InterfaceText *m_item2ActivatedText;  // 0x278
    safe_enum<Menu_Ingame::EStateMultipliersPerksDef,enum Menu_Ingame::EStateMultipliersPerksDef::type> m_stateBananaMP;  // 0x27C
    safe_enum<Menu_Ingame::EStateMultipliersPerksDef,enum Menu_Ingame::EStateMultipliersPerksDef::type> m_stateScoreMP;  // 0x280
    bool m_igNotification_canUpdateOrRender;  // 0x284
    float m_igNotification_perksRenderOffset;  // 0x288
    WeeklyMission *m_currentWeekMission;  // 0x28C
    InterfaceText *m_currentWeekMissionText;  // 0x290
    OnlineUser *m_nextFriendToBeat;  // 0x294
    InterfaceArea *m_nextFriendToBeatAvatarArea;  // 0x298
    InterfaceGraph *m_nextFriendToBeatBg;  // 0x29C
    InterfaceArea *m_nextFriendToBeatParent;  // 0x2A0
    int m_nextFriendScore;  // 0x2A4
    int m_nextFriendEffectTimer;  // 0x2A8
    static const int k_nextFriendOutTime;
    static const int k_nextFriendInTime;
};

#endif
