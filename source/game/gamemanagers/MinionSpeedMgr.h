// source/game/gamemanagers/MinionSpeedMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_MINIONSPEEDMGR_H
#define SOURCE_GAME_GAMEMANAGERS_MINIONSPEEDMGR_H

#include <vector>
#include "jsingleton.h"
#include "core/delayer.h"
#include "core/pstring.h"

// How fast the minion runs along the level: the speed definition of the minion lists intervals (a number of rooms and the speed the run
// has at their end); the speed eases from one interval's to the next over the rooms of the first, at the acceleration that makes it
// reach it in the distance. A room is k_roomSize long and the level templates passed add to the distance. A script can force a speed.
// 0x40 bytes in the .so.
class MinionSpeedMgr : public Singleton<MinionSpeedMgr> {
public:
    struct SpeedInterval { int roomAmount; float speed; };
    MinionSpeedMgr();
    virtual ~MinionSpeedMgr();
    void PostInit();
    void Update(unsigned dt);
    // a level template of the run was passed: its length counts for the rooms
    void OnAdvanceLevelTemplateList();
    // @0x57b578 (Minion::Revive): the run goes on at the speed of the interval the data says
    void OnMinionRevive();
    // the interval the run goes on with after the minion was revived
    int GetSpeedIntervalOnRevive() const { return m_speedIntervalOnRevive; }
    float GetCurrentSpeed() const;
    void SetSpeedForcedByScript(bool forced, float speed) { m_forceSpeedByScript = forced; m_speedByScript = speed; }
    static const float k_roomSize;
    // the speed from which the distance between the items grows (GameItem::GetNextDistanceSpace, @0xc8ae38)
    static const float k_minimumSpeedForIncreaseFactor;
    // the speed the delayer has now, even when a script forces another (the items read it, @0x4f61dc reads the other)
    float GetDelayedSpeed() const { return m_speedDelayer.GetCurrentValue(); }
private:
    void SetSpeedDelayer();
    jet::core::Delayer<float, jet::core::LinearStep> m_speedDelayer;  // 0x4
    std::vector<SpeedInterval*> m_speedIntervals;                    // 0x1C
    int m_distance;                                                  // 0x28: how far into the room the run is
    unsigned m_roomsRunned;                                          // 0x2C: the rooms of the current interval done
    int m_currentInterval;                                           // 0x30
    int m_speedIntervalOnRevive;                                     // 0x34
    bool m_forceSpeedByScript;                                       // 0x38
    float m_speedByScript;                                           // 0x3C
    static const jet::String k_speedIntervalsParam, k_roomAmountParam, k_speedParam, k_speedIntervalOnReviveParam;
};

#endif
