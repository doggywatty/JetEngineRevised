// source/game/cheats/CheatsEntry.cpp
#include "CheatsEntry.h"

// @0x591bac
CheatsEntry::CheatsEntry(const jet::String &name, bool enabled) : m_name(name), m_enabled(false) { SetEnabled(enabled); }

// @0x591ec4
CheatsEntry::~CheatsEntry() {}

// @0x592324: the state is kept; the menu's text of the entry (InterfaceText::SetText with "yes" or "no") is not ported with the menu
void CheatsEntry::SetEnabled(bool enabled) { m_enabled = enabled; }
