// source/game/gamemanagers/GameSoundMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_GAMESOUNDMGR_H
#define SOURCE_GAME_GAMEMANAGERS_GAMESOUNDMGR_H

#include <list>
#include <map>
#include "core/pstring.h"
#include "jsingleton.h"
#include "smath.h"

// The sounds of the game that have rules of their own: a sound played after a delay, a sound that does not play again for a time
// (its cooldown), and the sound of the bananas, which goes up as they are collected one after the other (a combo).
class GameSoundMgr : public Singleton<GameSoundMgr> {  // sizeof 0x44
public:
    struct DelayedSound {
        jet::String label;  // 0x8 in the list's node
        float time;  // 0xC: what is left of the delay, in the milliseconds of the frames
    };
    struct PlayedSoundInfo {
        PlayedSoundInfo() : cooldown(0), handle(-1) {}
        float cooldown;  // the milliseconds before the sound can be played again
        int handle;  // the sound of SoundMgr
    };
    GameSoundMgr();
    virtual ~GameSoundMgr();
    void Init();
    void ClearDelayedSounds();
    void Update();
    void UpdateDelayedSounds();
    void UpdateSoundInfo();
    void PlaySoundDelayed(const jet::String&, float);
    int Play(const jet::String&, float, const math::vec3<float>&, unsigned);
    void ResetCoinCollectedSound();
    void UpdateCollideCoindSound();
    void PlayCollideCoinSound(const jet::String&, const math::vec3<float>&);
private:
    std::list<DelayedSound> m_delayedSounds;  // 0x4
    std::map<jet::String, PlayedSoundInfo> m_playedSounds;  // 0xC
    int m_lastCoindCollectedTimer;  // 0x24: the milliseconds since the last banana
    int m_coinCollectedCombo;  // 0x28
    int m_coinCollectedHappyCombo;  // 0x2C
    static const float k_collideCoinCooldownSound;
    static const int k_bananaInComboTime;
    static const int k_bananaInGroupTime;
    static const int k_bananGroupNumber;
    static const int k_bananaHappyComboCount;
    static const jet::String k_bananaComboLabel;
    static const jet::String k_bananaHappyComboLabel;
};

#endif
