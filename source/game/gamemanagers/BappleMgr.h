// source/game/gamemanagers/BappleMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_BAPPLEMGR_H
#define SOURCE_GAME_GAMEMANAGERS_BAPPLEMGR_H

#include <vector>
#include "jsingleton.h"
#include "entity.h"

// Where a bapple objective is played: the locations of the run (the lab, the house, the macho lab, the beach and the mall), then others
int LocationToBappleSpot(int location);
int EdgeConditionToBappleSpot(int condition);

// A bapple to find (the rest of the class comes with the bapple hunt)
class BappleObjective : public clara::DataEntity {
public:
    explicit BappleObjective(const clara::Template *t) : clara::DataEntity(t), m_completed(false), m_spot(-1) {}
    bool IsCompleted() const { return m_completed; }
    int GetSpot() const { return m_spot; }
private:
    bool m_completed;  // 0x20
    int m_spot;        // 0x28
};

// The bapple hunt of the day. In this version it is never started (TryToCreateBapple makes nothing), so the objectives
// the level looks at are the ones the player activated: none until their activation is ported.
class BappleMgr : public Singleton<BappleMgr> {
public:
    virtual ~BappleMgr() {}
    // whether an objective of the spot is active and not done
    bool AreActiveObjectivesInSpot(int spot) const;
    bool IsBappleHuntUnlocked() const { return m_huntUnlocked; }
    void ComputeBappleHuntUnlock() {}  // @0x543e5c: nothing in this version
    // entering a spot resets its objectives
    void OnEnterSpot(int spot);
private:
    std::vector<BappleObjective*> m_activeObjectives;  // 0x64
    bool m_huntUnlocked = false;                       // 0x2C
};

#endif
