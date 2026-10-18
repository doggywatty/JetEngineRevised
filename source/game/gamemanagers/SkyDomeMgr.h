// source/game/gamemanagers/SkyDomeMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_SKYDOMEMGR_H
#define SOURCE_GAME_GAMEMANAGERS_SKYDOMEMGR_H

#include "jsingleton.h"
#include "core/pstring.h"

class Deco3d;

// The sky dome following the minion: a copy of the deco the background names, replaced when another is asked for
class SkyDomeMgr : public Singleton<SkyDomeMgr> {  // sizeof 0xC
public:
    SkyDomeMgr() : m_dome(0) {}
    virtual ~SkyDomeMgr() { CleanOldSkyDome(); }
    void AddSkyDome(const Deco3d &deco);
    void CleanOldSkyDome();
    Deco3d *GetDome() const { return m_dome; }
private:
    Deco3d *m_dome;          // 0x4
    jet::String m_name;      // 0x8
};

#endif
