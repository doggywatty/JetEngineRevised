// source/game/gamemanagers/PowerUpMgr.cpp
#include "PowerUpMgr.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../entities/Minion.h"

bool PowerUpMgr::s_isPowerUpActive = false;

// @0x4f91cc: seven power ups, all off
PowerUpMgr::PowerUpMgr() : m_activePerks(0), m_status(EPowerUpDef::COUNT) {
    for (PowerUpStatus &s : m_status) s.timeLeft = s.totalTime = 0;
}

// @0x4f92ec
PowerUpMgr::~PowerUpMgr() {}

// @0x4f93dc
void PowerUpMgr::ResetAll() {
    for (int i = 0; i < EPowerUpDef::COUNT; i++) CancelPowerUp(EPowerUp(i));
}

// @0x4f9404
void PowerUpMgr::DisableAll(bool notify) {
    for (int i = 0; i < EPowerUpDef::COUNT; i++) DisablePowerUp(EPowerUp(i), notify);
}

// @0x4f9434
void PowerUpMgr::DisableAllButOne(EPowerUp keep, bool notify) {
    for (int i = 0; i < EPowerUpDef::COUNT; i++)
        if (i != (int)keep.underlying()) DisablePowerUp(EPowerUp(i), notify);
}

// @0x4f9470: the time runs down; one that has none left goes off
void PowerUpMgr::Update() {
    for (int i = 0; i < EPowerUpDef::COUNT; i++) {
        const int left = m_status[(unsigned)i].timeLeft;
        if (left <= 0) continue;
        const int now = left - (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
        if (now > 0)
            m_status[(unsigned)i].timeLeft = now;
        else
            DisablePowerUp(EPowerUp(i), true);
    }
}

// @0x4f94ec: a power up that was off tells the minion it starts; giving one the time turns the others off
void PowerUpMgr::EnablePowerUp(EPowerUp p, int ms) {
    PowerUpStatus &s = m_status[(unsigned)p.underlying()];
    if (s.timeLeft == 0) OnPowerUpEnabled(p);
    s.timeLeft = ms;
    s.totalTime = ms;
    DisableAllButOne(p, false);
}

// @0x4f953c
void PowerUpMgr::DisablePowerUp(EPowerUp p, bool notify) {
    if (m_status[(unsigned)p.underlying()].timeLeft != 0) {
        if (GameLevel *level = GameLevel::GetInstance())
            if (Minion *m = level->GetMinion()) m->OnPowerUpDisabled(p, notify);
        s_isPowerUpActive = false;
    }
    CancelPowerUp(p);
}

// @0x4f95b0
void PowerUpMgr::CancelPowerUp(EPowerUp p) { m_status[(unsigned)p.underlying()].timeLeft = 0; }

// @0x4f95c0
void PowerUpMgr::OnPowerUpEnabled(EPowerUp p) {
    if (GameLevel *level = GameLevel::GetInstance())
        if (Minion *m = level->GetMinion()) m->OnPowerUpEnabled(p);
    s_isPowerUpActive = true;
}

// @0x4f964c
jet::String PowerUpMgr::GetPowerUpStringId(EPowerUp p) {
    static const char *const names[EPowerUpDef::COUNT] = {"POWERUP_MAGNET",           "POWERUP_GODMODE",  "POWERUP_INVINCIBLE", "POWERUP_SHIELD",
                                                          "POWERUP_BANANAMULTIPLIER", "POWERUP_DESPICABLEMULTIPLIER", "POWERUP_WEAPON"};
    return jet::String(names[p.underlying()]);
}
