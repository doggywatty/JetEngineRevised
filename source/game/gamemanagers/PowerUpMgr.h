// source/game/gamemanagers/PowerUpMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_POWERUPMGR_H
#define SOURCE_GAME_GAMEMANAGERS_POWERUPMGR_H

#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "../gameenums.h"

// The power ups that are on during a run, each with the time (ms) it has left and the time it was given (0 left: it is off), and the perks
// the player carries into the run. 0x18 bytes in the .so. The minion is told when one starts and stops (Minion::OnPowerUpEnabled and
// OnPowerUpDisabled).
class PowerUpMgr : public Singleton<PowerUpMgr> {
public:
    struct PowerUpStatus { int timeLeft, totalTime; };
    // the perks the player has bought for the run
    // the bits of the .so (Menu_Ingame::_UseBananaPerk sets 4, _UseDespicableMultiplier 8)
    struct EPowerUpPerkDef { enum type { BananaMultiplier = 4, DespicableMultiplier = 8 }; };
    PowerUpMgr();
    virtual ~PowerUpMgr();
    void ResetAll();
    void ResetPerks() { m_activePerks = 0; }
    void DisableAll(bool notify);
    void DisableAllButOne(EPowerUp, bool notify);
    void Update();
    void EnablePowerUp(EPowerUp, int ms);
    void DisablePowerUp(EPowerUp, bool notify);
    void CancelPowerUp(EPowerUp);
    bool IsPowerUpActive(EPowerUp p) const { return m_status[(unsigned)p.underlying()].timeLeft != 0; }
    // the weapon (the freeze gun) is the last power up
    bool IsFreezeGunActive() const { return IsPowerUpActive(EPowerUpDef::Weapon); }
    void OnPowerUpEnabled(EPowerUp);
    const PowerUpStatus &GetPowerUpStatus(EPowerUp p) const { return m_status[(unsigned)p.underlying()]; }
    void SetPerk(unsigned perk, bool on) { m_activePerks = on ? (m_activePerks | perk) : (m_activePerks & ~perk); }
    bool HasPerk(unsigned perk) const { return (m_activePerks & perk) == perk; }
    static jet::String GetPowerUpStringId(EPowerUp);
    // a power up is on (GameItem::s_isPowerUpActive in the original: the game items are not ported)
    static bool s_isPowerUpActive;
private:
    unsigned m_activePerks;                 // 0x4
    std::vector<PowerUpStatus> m_status;    // 0x8
};

#endif
