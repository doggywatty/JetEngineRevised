// source/game/cheats/CheatsMgr.cpp
#include "CheatsMgr.h"

// @0x592420: the entry's name is "Invincible?" (the .so's string at 0x592898)
CheatsMgr::CheatsMgr() : m_areCheatsEnabled(false), m_invincibleCheatEntry(new CheatsEntry(jet::String("Invincible?"), false)) {}

// @0x59361c
CheatsMgr::~CheatsMgr() {}

// @0x55e080: the invincible cheat is on only while the cheats are enabled (the .so reads the entry's flag at +0x1E8 when the flag at +0x1C is set)
bool CheatsMgr::IsInvincible() const { return m_areCheatsEnabled && m_invincibleCheatEntry->IsEnabled(); }

// @0x595944: the entry is changed only while the cheats are enabled
void CheatsMgr::SetInvincible(bool on) {
    if (!m_areCheatsEnabled) return;
    m_invincibleCheatEntry->SetEnabled(on);
}
