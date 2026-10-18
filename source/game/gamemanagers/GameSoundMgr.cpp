// source/game/gamemanagers/GameSoundMgr.cpp
#include "GameSoundMgr.h"
#include <string>
#include <vector>
#include "game/CGameConfig.h"
#include "game/Game.h"
#include "sound/soundmgr.h"

// the values at @0xc8b444 (the cooldown of the banana sound, in ms) and the next four
const float GameSoundMgr::k_collideCoinCooldownSound = 50.0f;
const int GameSoundMgr::k_bananaInComboTime = 400;
const int GameSoundMgr::k_bananaInGroupTime = 200;
const int GameSoundMgr::k_bananGroupNumber = 4;
const int GameSoundMgr::k_bananaHappyComboCount = 5;
// the labels @0xd0ed8c and @0xd0ed90 (the initialiser @0x55a154)
const jet::String GameSoundMgr::k_bananaComboLabel("evt_banana");
const jet::String GameSoundMgr::k_bananaHappyComboLabel("evt_banana_low");

// @0x54b8b0
GameSoundMgr::GameSoundMgr() : m_lastCoindCollectedTimer(0), m_coinCollectedCombo(0), m_coinCollectedHappyCombo(0) {}

// @0x54b904
GameSoundMgr::~GameSoundMgr() {}

// @0x54b9c0: a device that sounds are too much for (the toggle of its profile) has the banks of the soundpack cut to 30%
void GameSoundMgr::Init() {
    if (!CGameConfig::Current().useLessSounds) return;
    std::vector<std::string> banks;
    banks.push_back("AMB");
    banks.push_back("AMB_LOOP");
    banks.push_back("IN GAME");
    banks.push_back("MC VO");
    banks.push_back("UNIQUE");
    banks.push_back("ENEMY VO");
    banks.push_back("UI");
    SoundMgr::GetInstance()->ReconfigureBanks(banks, 0.3f);
}

// @0x54bbb0
void GameSoundMgr::ClearDelayedSounds() { m_delayedSounds.clear(); }

// @0x54bbd0
void GameSoundMgr::Update() {
    UpdateDelayedSounds();
    UpdateSoundInfo();
    UpdateCollideCoindSound();
}

// @0x54bbf0: the sounds whose delay is over play
void GameSoundMgr::UpdateDelayedSounds() {
    const float dt = Game::GetInstance()->GetUpdateTimeAsFloat();
    for (std::list<DelayedSound>::iterator it = m_delayedSounds.begin(); it != m_delayedSounds.end();) {
        it->time -= dt;
        if (it->time > 0) { ++it; continue; }
        SoundMgr::GetInstance()->Play(it->label, 0);
        it = m_delayedSounds.erase(it);
    }
}

// @0x54bc8c: the cooldowns of the sounds that were played
void GameSoundMgr::UpdateSoundInfo() {
    const float dt = Game::GetInstance()->GetUpdateTimeAsFloat();
    for (std::map<jet::String, PlayedSoundInfo>::iterator it = m_playedSounds.begin(); it != m_playedSounds.end(); ++it)
        if (it->second.cooldown > 0) it->second.cooldown -= dt;
}

// @0x54bce4
void GameSoundMgr::PlaySoundDelayed(const jet::String &label, float delay) {
    if (delay > 0) {
        DelayedSound d;
        d.label = label;
        d.time = delay;
        m_delayedSounds.push_back(d);
    } else {
        SoundMgr::GetInstance()->Play(label, 0);
    }
}

// @0x54bd80: a sound with a cooldown is not played again before it is over
int GameSoundMgr::Play(const jet::String &label, float cooldown, const math::vec3<float> &position, unsigned fadeMs) {
    std::map<jet::String, PlayedSoundInfo>::iterator it = m_playedSounds.find(label);
    if (it != m_playedSounds.end() && it->second.cooldown > 0) return -1;
    const int handle = SoundMgr::GetInstance()->Play3D(label, position, fadeMs);
    if (cooldown > 0) {
        PlayedSoundInfo &info = m_playedSounds[label];
        info.cooldown = cooldown;
        info.handle = handle;
    }
    return handle;
}

// @0x54be80
void GameSoundMgr::ResetCoinCollectedSound() {
    m_lastCoindCollectedTimer = 0;
    m_coinCollectedCombo = 0;
    m_coinCollectedHappyCombo = 0;
}

// @0x54be94
void GameSoundMgr::UpdateCollideCoindSound() { m_lastCoindCollectedTimer += (int)Game::GetInstance()->GetUpdateTime(); }

// @0x54bed4: bananas collected one after the other play the next sound of the event; after a pause the event starts again, and
// the fifth of a quick run plays the "happy" one too
void GameSoundMgr::PlayCollideCoinSound(const jet::String &label, const math::vec3<float> &position) {
    bool reset = false;
    if (m_lastCoindCollectedTimer <= k_bananaInGroupTime) {
        if (m_coinCollectedHappyCombo < k_bananaHappyComboCount) {
            m_coinCollectedHappyCombo++;
            if (m_coinCollectedHappyCombo == k_bananaHappyComboCount) Play(k_bananaHappyComboLabel, k_collideCoinCooldownSound, math::vec3<float>(0, 0, 0), 0);
        }
    } else {
        m_coinCollectedCombo = 0;
        reset = m_lastCoindCollectedTimer > k_bananaInComboTime;
        m_coinCollectedHappyCombo = 0;
    }
    m_lastCoindCollectedTimer = 0;
    if (reset) SoundMgr::GetInstance()->ResetEvent(label);
    Play(label, k_collideCoinCooldownSound, position, 0);
}
