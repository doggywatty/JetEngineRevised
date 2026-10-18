// source/game/input/InputMgr.h
#ifndef SOURCE_GAME_INPUT_INPUTMGR_H
#define SOURCE_GAME_INPUT_INPUTMGR_H

// Same zero-includes PDB-extraction gap as every other header this session has touched.
#include <bitset>
#include "smath.h"
#include "jsingleton.h"
class Minion;

// Singleton<InputMgr> is this session's choice, not a verified fact about the real binary -- the real
// game accesses this via free functions (glf::App::GetInputMgr()/glf::GetInputMgr(), part of an app
// framework layer this tree doesn't have), and nothing anywhere in this tree constructed or updated an
// InputMgr before this pass. Every other per-game-singleton manager here (MenuMgr, GameSettings,
// PopupMgr, ...) already uses Singleton<T> regardless of how the real binary accessed it, so this follows
// that established local convention rather than trying to rebuild the missing glf::App layer.
class InputMgr : public Singleton<InputMgr> {  // sizeof 0x60
public:
    // Not in the PDB header. GuiObject inherits TutorialInputListener (its vtable is 12 bytes =
    // complete+deleting dtor + one virtual method), and every real override found this session
    // (GameLevel::OnTutorialInputDone, Minion::OnTutorialInputDone, InputMgr's own private one) takes a
    // `const TutorialInputId&`, consistent with a small callback-id/listener pair. TutorialInputId's
    // real shape (plain enum vs wrapped id) isn't verified -- this is a placeholder, flagged as such,
    // same spirit as Menu_SendChallengeTaunt::STauntElement earlier this session.
    // id 0 is "no tutorial step" (IsTutorialActionEnabled @0x5049fc returns true for it; BossMeena clears the step with TutorialInputId{0}).
    // The .so's InputMgr::InputMgr (@0x5045a8) never writes m_tutorialId (+0x4), so the no-step value is 0, not -1: with -1 every
    // swipe to the sides and the jump is refused by IsTutorialActionEnabled.
    struct TutorialInputId { int id = 0; bool operator==(const TutorialInputId &o) const { return id == o.id; } };
    class TutorialInputListener {
    public:
        virtual ~TutorialInputListener() {}
        virtual void OnTutorialInputDone(const TutorialInputId &) {}
    };

    InputMgr();
    ~InputMgr();
    void HandleAccelerometer(float, float);
    void SetEnabled(bool);
    void SetTutorial(InputMgr::TutorialInputId, InputMgr::TutorialInputListener*);
    virtual void Update(int);
    virtual void UpdateSlideActions(int);
    virtual void LaunchEvents();
    void Reset();
    void SetPlayer(Minion*);
    void OnSlideAction(int);
    bool IsSlideAction(int);
    virtual void OnKeyPressed(int);
    virtual void OnKeyReleased(int);
    virtual int GetTouchAction(int);
    void EnableTouchInteracts(bool);
    bool IsTouchInteractsEnabled();
    void IgnoreAllSlideActions(bool);
    void IgnoreVerticalSlideActions(bool);
    float GetInputRatio();
    float GetInputRatioLast10Seconds();
protected:
    int GetLineSlideAction8(const math::vec2<float>&, const math::vec2<float>&);
    int GetLineSlideAction4(const math::vec2<float>&, const math::vec2<float>&);
    bool IsTutorialActionEnabled(int);
private:
    void OnTutorialInputDone(const InputMgr::TutorialInputId&);
public:
    static const float k_SLIDE_MIN_DISTANCE;
    static const float k_SLIDE_MAX_DISTANCE;
    static const float k_TOUCH_MAX_DISTANCE;
    static const int k_SLIDE_ACTION_UpDown;
    static const int k_SLIDE_ACTION_UpRightDownLeft;
    static const int k_SLIDE_ACTION_RightLeft;
    static const int k_SLIDE_ACTION_DownRightUpLeft;
    static const int k_SLIDE_ACTION_DownUp;
    static const int k_SLIDE_ACTION_DownLeftUpRight;
    static const int k_SLIDE_ACTION_LeftRight;
    static const int k_SLIDE_ACTION_UpLeftDownRight;
    static const int k_MAX_SLIDE_ACTIONS;
private:
    InputMgr::TutorialInputId m_tutorialId;  // 0x4
    InputMgr::TutorialInputListener *m_tutorialDoneListener;  // 0x8
    bool m_isEnabled;  // 0xC
    bool m_enableTouchInteracts;  // 0xD
    int m_currentTouch;  // 0x10
    math::vec2<float> m_touchInitialPos;  // 0x14
    math::vec2<float> m_touchLastPos;  // 0x1C
    int m_pressedTime;  // 0x24
    int m_oldSlideAction;  // 0x28
    int m_upDirection;  // 0x2C
    std::bitset<8> m_slideActions;  // 0x30
    int m_keyboardSlideAction;  // 0x34
    static float s_minSlideDistance;
    static float s_maxSlideDistance;
    static float s_maxTouchDistance;
    static bool s_isInit;
    bool m_touched;  // 0x38
    math::vec2<float> m_touchedPos;  // 0x3C
    Minion *m_player;  // 0x44
    bool m_ignoreAllSlideActions;  // 0x48
    bool m_ignoreVerticalSlideActions;  // 0x49
    float m_inputRatio;  // 0x4C
    unsigned m_inputRatioCount;  // 0x50
    float m_inputRatioLast10Seconds;  // 0x54
    unsigned m_inputLast10secCount;  // 0x58
    unsigned m_inputLast10secTimeCount;  // 0x5C
};

#endif
