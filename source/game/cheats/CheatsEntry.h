// source/game/cheats/CheatsEntry.h
#ifndef SOURCE_GAME_CHEATS_CHEATSENTRY_H
#define SOURCE_GAME_CHEATS_CHEATSENTRY_H

#include "core/pstring.h"

// One cheat of the cheats menu: its name and its on/off state. The .so's entry (0x228 bytes, ctor @0x591bac) also holds the menu's text, button,
// callback and child entries; only the name and the state the gameplay reads are ported here (the menu is not ported).
class CheatsEntry {
public:
    CheatsEntry(const jet::String &name, bool enabled);  // @0x591bac
    virtual ~CheatsEntry();                              // @0x591ec4
    const jet::String &GetName() const { return m_name; }
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);                       // @0x592324
private:
    jet::String m_name;  // 0x1E4
    bool m_enabled;      // 0x1E8
};

#endif
