// source/game/input/InputMgr.cpp
//
// Port of the player's swipe/touch control scheme. The octant classifiers (GetLineSlideAction8/4) and
// now UpdateSlideActions (the real per-frame touch-tracking state machine, reading TouchMgr -- already
// real in this tree from an earlier session) are fully verified against the disassembly, including every
// boundary-angle/threshold constant and the k_SLIDE_ACTION_* mapping (cross-checked against real .data
// values). LaunchEvents() -- the event-dispatch half, which reads the slide-action bitset this now
// produces and is meant to call into Minion's lane-change/jump/slide methods -- is not ported yet; see
// its own comment below for why and what's needed next.
#include "InputMgr.h"
#include "system.h"
#include "video/display.h"
#include "TouchMgr.h"
#include "camera/cameramgr.h"
#include "scene/scenemgr.h"
#include "../entities/Minion.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

const float InputMgr::k_SLIDE_MIN_DISTANCE = 28.0f;
const float InputMgr::k_SLIDE_MAX_DISTANCE = 60.0f;
const float InputMgr::k_TOUCH_MAX_DISTANCE = 30.0f;
// Verified: real .data values at each symbol, not assumed from declaration order.
const int InputMgr::k_SLIDE_ACTION_UpDown = 0;
const int InputMgr::k_SLIDE_ACTION_UpRightDownLeft = 1;
const int InputMgr::k_SLIDE_ACTION_RightLeft = 2;
const int InputMgr::k_SLIDE_ACTION_DownRightUpLeft = 3;
const int InputMgr::k_SLIDE_ACTION_DownUp = 4;
const int InputMgr::k_SLIDE_ACTION_DownLeftUpRight = 5;
const int InputMgr::k_SLIDE_ACTION_LeftRight = 6;
const int InputMgr::k_SLIDE_ACTION_UpLeftDownRight = 7;
const int InputMgr::k_MAX_SLIDE_ACTIONS = 8;

float InputMgr::s_minSlideDistance = 0.0f;
float InputMgr::s_maxSlideDistance = 0.0f;
float InputMgr::s_maxTouchDistance = 0.0f;
bool InputMgr::s_isInit = false;

// Verified @0x5045a8: the three k_SLIDE_*_DISTANCE constants are percentages of a 768-unit reference
// width (28/768, 60/768, 30/768 match the real literal pool exactly), scaled by the real screen width and
// squared (so callers compare squared distances, avoiding a sqrt) -- computed once, guarded by s_isInit.
InputMgr::InputMgr()
    : m_tutorialId(), m_tutorialDoneListener(0), m_isEnabled(true), m_enableTouchInteracts(false),
      m_currentTouch(-1), m_touchInitialPos(0, 0), m_touchLastPos(0, 0), m_pressedTime(0), m_oldSlideAction(-1),
      m_upDirection(0), m_keyboardSlideAction(0), m_touched(false), m_touchedPos(0, 0), m_player(0),
      m_ignoreAllSlideActions(false), m_ignoreVerticalSlideActions(false), m_inputRatio(0.0f),
      m_inputRatioCount(0), m_inputRatioLast10Seconds(0.0f), m_inputLast10secCount(0), m_inputLast10secTimeCount(0) {
    if (!s_isInit) {
        s_isInit = true;
        jet::video::Display *disp = jet::System::GetMainDisplay();
        float w = disp ? (float)disp->GetMode().size.x : 768.0f;
        float minD = 28.0f / 768.0f * w, maxD = 60.0f / 768.0f * w, touchD = 30.0f / 768.0f * w;
        s_minSlideDistance = minD * minD;
        s_maxSlideDistance = maxD * maxD;
        s_maxTouchDistance = touchD * touchD;
    }
}

InputMgr::~InputMgr() {}

// Clears per-touch tracking state. Real offsets this reads/writes couldn't be cleanly mapped back to
// this header's field layout one-for-one (same kind of shift seen on other headers this session) --
// reproduced by intent (clear the touch/slide tracking, not a from-scratch guess) rather than by offset.
// @0x5046c8: the keyboard slide, the touch state and the slide direction go back to their untouched values
void InputMgr::Reset() {
    m_keyboardSlideAction = 0;
    m_touched = false;
    m_currentTouch = -1;
    m_oldSlideAction = 0;
    m_upDirection = -1;
}

void InputMgr::SetEnabled(bool v) { m_isEnabled = v; }
void InputMgr::SetPlayer(Minion *m) { m_player = m; }
void InputMgr::EnableTouchInteracts(bool v) { m_enableTouchInteracts = v; }
bool InputMgr::IsTouchInteractsEnabled() { return m_enableTouchInteracts; }
void InputMgr::IgnoreAllSlideActions(bool v) { m_ignoreAllSlideActions = v; }
void InputMgr::IgnoreVerticalSlideActions(bool v) { m_ignoreVerticalSlideActions = v; }
float InputMgr::GetInputRatio() { return m_inputRatio; }
float InputMgr::GetInputRatioLast10Seconds() { return m_inputRatioLast10Seconds; }

// Verified @0x4f4fdc: tests bit `action` of the slide-actions bitset.
bool InputMgr::IsSlideAction(int action) { return action >= 0 && action < 8 && m_slideActions.test(action); }

// Verified @0x4f4fd4: the base implementation just returns "no touch action" (-1); real per-touch
// handling (if any override exists) isn't ported.
int InputMgr::GetTouchAction(int) { return -1; }

void InputMgr::OnKeyPressed(int) {}
void InputMgr::OnKeyReleased(int) {}

// Verified @0x504df8: classic 8-octant swipe classifier. angle = atan2f(p2.y - p1.y, p2.x - p1.x),
// normalized to [0, 2*PI), then bucketed into 8 PI/4-wide sectors offset by PI/8 so each named direction
// is centered on its compass angle. Every boundary (PI/8, 3PI/8, 5PI/8, 7PI/8, 9PI/8, 11PI/8, 13PI/8,
// 15PI/8) and the LeftRight/UpLeftDownRight/UpDown/UpRightDownLeft/RightLeft/DownRightUpLeft/DownUp/
// DownLeftUpRight return-value sequence is read directly from the literal pool and cross-checked against
// the real k_SLIDE_ACTION_* data above, not assumed.
int InputMgr::GetLineSlideAction8(const math::vec2<float> &p1, const math::vec2<float> &p2) {
    float dy = p2.y - p1.y, dx = p2.x - p1.x;
    float a = atan2f(dy, dx);
    if (a < 0) a += 6.2831855f;
    if (a >= 5.8904862f || a < 0.3926991f) return k_SLIDE_ACTION_LeftRight;
    if (a < 1.1780972f) return k_SLIDE_ACTION_UpLeftDownRight;
    if (a < 1.9634955f) return k_SLIDE_ACTION_UpDown;
    if (a < 2.7488937f) return k_SLIDE_ACTION_UpRightDownLeft;
    if (a < 3.5342917f) return k_SLIDE_ACTION_RightLeft;
    if (a < 4.3196902f) return k_SLIDE_ACTION_DownRightUpLeft;
    if (a < 5.1050882f) return k_SLIDE_ACTION_DownUp;
    return k_SLIDE_ACTION_DownLeftUpRight;
}

// Verified @0x504f10: same shape, 4 quadrants (boundaries PI/4, 3PI/4, 5PI/4, 7PI/4).
int InputMgr::GetLineSlideAction4(const math::vec2<float> &p1, const math::vec2<float> &p2) {
    float dy = p2.y - p1.y, dx = p2.x - p1.x;
    float a = atan2f(dy, dx);
    if (a < 0) a += 6.2831855f;
    if (a >= 5.4977875f || a < 0.7853982f) return k_SLIDE_ACTION_LeftRight;
    if (a < 2.3561945f) return k_SLIDE_ACTION_UpDown;
    if (a < 3.9269910f) return k_SLIDE_ACTION_RightLeft;
    return k_SLIDE_ACTION_DownUp;
}

// @0x5049fc: while a tutorial step waits for an input, only the swipes of the step count: 1 and 3 the swipes to the sides' left-right, 2 and 3 the other
// way, 4 up, 5 down
void InputMgr::SetTutorial(InputMgr::TutorialInputId id, InputMgr::TutorialInputListener *l) { m_tutorialId = id; m_tutorialDoneListener = l; }
bool InputMgr::IsTutorialActionEnabled(int action) {
    const int id = m_tutorialId.id;
    if (id == 0) return true;
    switch (action) {
    case 0: return id == 5;
    case 2: return (unsigned)(id - 2) <= 1;
    case 4: return id == 4;
    case 6: return (id & ~2) == 1;
    default: return true;
    }
}
void InputMgr::OnTutorialInputDone(const InputMgr::TutorialInputId &id) { if (m_tutorialDoneListener) m_tutorialDoneListener->OnTutorialInputDone(id); }

// Verified @0x5046e8: if disabled and no tutorial listener is waiting, do nothing; otherwise run the
// slide-action detection then dispatch events. dt is passed straight through to UpdateSlideActions.
void InputMgr::Update(int dt) {
    if (!m_isEnabled && !m_tutorialDoneListener) return;
    UpdateSlideActions(dt);
    LaunchEvents();
}

// @0x50472c: the touches of the frame. The first touch pressed is the one followed; once it is farther from where it began than the
// slide distance (or is released) it is a swipe if it went farther than the minimum: its direction (to the sides, up or down) is the
// action of the frame, once for the touch, and which way on the vertical it went is kept. A touch that stays near where it began is a touch
// (for the things on the screen).
void InputMgr::UpdateSlideActions(int dt) {
    m_slideActions.reset();
    m_upDirection = -1;
    m_touched = false;

    std::vector<const Touch*> touches = TouchMgr::GetInstance()->GetAllActiveTouches();
    if (touches.empty()) {
        m_currentTouch = -1;
        m_pressedTime = 0;
        m_oldSlideAction = -1;
        return;
    }
    for (const Touch *t : touches) {
        if ((t->state & (Touch::Pressed | Touch::Dragged)) && m_currentTouch == -1) {
            m_currentTouch = t->id;
            m_pressedTime = 0;
            m_touchInitialPos = t->pos;
        }
        const float distSq = math::distanceSq(m_touchInitialPos, t->pos);
        m_pressedTime += dt;
        m_touchLastPos = t->pos;
        const bool released = (t->state & Touch::Released) != 0;
        if (!released && distSq < s_maxSlideDistance) continue;
        if (m_currentTouch != t->id) continue;
        if (distSq > s_minSlideDistance) {
            const int action = GetLineSlideAction4(m_touchInitialPos, t->pos);
            if (action >= 0 && m_oldSlideAction == -1) {
                m_slideActions.set((size_t)action);
                m_oldSlideAction = action;
                m_upDirection = m_touchInitialPos.y < t->pos.y ? k_SLIDE_ACTION_UpDown : k_SLIDE_ACTION_DownUp;
            }
            m_pressedTime = 0;
        }
        if (distSq < s_maxTouchDistance) {
            m_touched = true;
            m_touchedPos = t->pos;
        }
        if (released) {
            m_currentTouch = -1;
            m_pressedTime = 0;
            m_oldSlideAction = -1;
        }
    }
}

// @0x504a70: what the swipes of the frame do to the minion. A swipe to the sides is a screen event (code 0, param 6 for the way the minion's
// right is and 2 for the other, as the camera sees it); one down is the slide (engine event 0x25) and one up the jump (9). A step of a
// tutorial that waits for the input is told when it is made. (A touch on the minion's body parts is not ported.)
void InputMgr::LaunchEvents() {
    if (getenv("JET_DBGINPUT") && m_slideActions.any()) fprintf(stderr, "slide bits %lx up %d player %p tutorial %d ignore %d\n", m_slideActions.to_ulong(), m_upDirection, (void*)m_player, m_tutorialId.id, (int)m_ignoreAllSlideActions);
    if (!m_player) return;
    if (!m_ignoreAllSlideActions) {
        const math::vec3<float> right = m_player->GetRotation() * jet::scene::SceneMgr::GetRightVector();
        const bool flipped = math::dot(right, CameraMgr::GetInstance()->GetSceneCamera()->GetRightVector()) < 0;
        bool launched = false;
        if (IsSlideAction(k_SLIDE_ACTION_LeftRight) && IsTutorialActionEnabled(k_SLIDE_ACTION_LeftRight)) {
            m_player->SM_OnScreenEvent(0, flipped ? 2 : 6, true);
            launched = true;
        } else if (IsSlideAction(k_SLIDE_ACTION_RightLeft) && IsTutorialActionEnabled(k_SLIDE_ACTION_RightLeft)) {
            m_player->SM_OnScreenEvent(0, flipped ? 6 : 2, true);
            launched = true;
        } else if (!m_ignoreVerticalSlideActions) {
            if ((IsSlideAction(k_SLIDE_ACTION_UpDown) || m_upDirection == k_SLIDE_ACTION_UpDown) && IsTutorialActionEnabled(k_SLIDE_ACTION_UpDown)) {
                m_player->SM_OnEngineEvent(0x25);
                launched = true;
            } else if ((IsSlideAction(k_SLIDE_ACTION_DownUp) || m_upDirection == k_SLIDE_ACTION_DownUp) && IsTutorialActionEnabled(k_SLIDE_ACTION_DownUp)) {
                m_player->SM_OnEngineEvent(9);
                launched = true;
            }
        }
        if (launched && m_tutorialId.id != 0 && m_tutorialDoneListener) OnTutorialInputDone(m_tutorialId);
    }
    if (m_tutorialId.id == 6 && m_touched && !m_enableTouchInteracts && m_tutorialDoneListener) OnTutorialInputDone(m_tutorialId);
    if (m_currentTouch != -1) m_player->HandleHoldInput();
}

void InputMgr::HandleAccelerometer(float, float) {}
void InputMgr::OnSlideAction(int) {}
