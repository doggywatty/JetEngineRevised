// source/game/cheats/CheatsMgr.h
#ifndef SOURCE_GAME_CHEATS_CHEATSMGR_H
#define SOURCE_GAME_CHEATS_CHEATSMGR_H

#include <boost/shared_ptr.hpp>
#include "jsingleton.h"
#include "CheatsEntry.h"

// The cheat state the gameplay reads: the cheats-enabled flag and the invincible cheat (the boss's ram checks it through IsInvincible). The rest
// of the .so's CheatsMgr (0xB4 bytes: its menu and its other cheats, ctor @0x592420) is not ported. The flag is written by the menu's "Enable
// cheats?" entry (Event_CheatsEnabled @0x59570c), which is not ported, so it stays off.
class CheatsMgr : public Singleton<CheatsMgr> {
public:
    CheatsMgr();           // @0x592420 (the flag at +0x1C off, the invincible entry at +0x3C off)
    virtual ~CheatsMgr();  // @0x59361c
    // The death revive incentives' cheat overrides (the .so's CheatsMgr::IsDRI*: each reads an entry of the debug menu when the cheats are on).
    // The cheats are off in the port (m_areCheatsEnabled stays false) and the debug menu's entries are not ported, so none is forced on.
    bool IsDRIScoreEntryActive() const { return false; }
    bool IsDRISocialChallengeEntryActive() const { return false; }
    bool IsDRITopFriendEntryActive() const { return false; }
    bool IsDRINextFriendEntryActive() const { return false; }
    bool IsDRIMissionEntryActive() const { return false; }
    bool IsInvincible() const;
    void SetInvincible(bool on);
    // The cheats-enabled flag (+0x1C). The .so writes it from the menu's "Enable cheats?" entry (@0x59570c, not ported); the test harness
    // (JET_INVINCIBLE=1) is the only caller here, so the shipped build keeps it off.
    void SetCheatsEnabled(bool on) { m_areCheatsEnabled = on; }
private:
    bool m_areCheatsEnabled;                                 // 0x1C
    boost::shared_ptr<CheatsEntry> m_invincibleCheatEntry;   // 0x3C
};

#endif
