// source/game/gamemanagers/EdgeProbabilityMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_EDGEPROBABILITYMGR_H
#define SOURCE_GAME_GAMEMANAGERS_EDGEPROBABILITYMGR_H

#include <vector>
#include "game/gamemanagers/EdgeMgr.h"

namespace clara { class DataEntity; }

// A probability set the data names ("ProbabilityEvolutionSet"): it starts at a value and grows by an increment every few rooms run,
// follows the missions that give the edge's reward and may wait for a boss
class EdgeProbabilityMgr : public EdgeMgr {  // sizeof 0x44
public:
    struct ProbabilityInterval {
        int roomAmount;   // 0x0
        float increment;  // 0x4
    };
    static const jet::String k_ProbabilityEvolutionSet_tmplName, k_ProbabilityIntervalDef_tmplName;
    EdgeProbabilityMgr() : m_initialProbability(0), m_hasMission(false), m_distanceRunned(0), m_roomsRunned(0), m_currentInterval(0),
        m_isLocationDependent(false), m_location(0), m_bossDependent(false), m_bossLocation(0) {}
    virtual ~EdgeProbabilityMgr();
    virtual void Create(const jet::String &name, LevelSequenceParser &parser);
    virtual bool Evaluate();
    virtual void Reset();
    virtual void ResetData();
    virtual void OnAdvanceLevelTemplateList();
    float GetCurrentProbability() const;
private:
    bool IsAnyMissionActive() const;
    bool IsAnyMissionDone() const;
    bool IsBossBeaten() const;
    std::vector<ProbabilityInterval*> m_probabilityIntervals;  // 0x4
    float m_initialProbability;                // 0x10
    bool m_hasMission;                         // 0x14
    std::vector<clara::DataEntity*> m_missions;  // 0x18
    unsigned m_distanceRunned;                 // 0x28
    unsigned m_roomsRunned;                    // 0x2C
    int m_currentInterval;                     // 0x30
    bool m_isLocationDependent;                // 0x34
    int m_location;                            // 0x38
    bool m_bossDependent;                      // 0x3C
    int m_bossLocation;                        // 0x40
};

#endif
