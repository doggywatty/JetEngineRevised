// source/game/entities/MissionSet.h
#ifndef SOURCE_GAME_ENTITIES_MISSIONSET_H
#define SOURCE_GAME_ENTITIES_MISSIONSET_H

#include <vector>
#include "entity.h"
#include "core/pstring.h"

class Mission;

// A list of missions the player does one after the other ("MissionSet" data entity, 0x2d bytes in the .so)
class MissionSet : public clara::DataEntity {
public:
    explicit MissionSet(const clara::Template*);
    virtual ~MissionSet();
    virtual void Init() {}
    virtual void PostInit();
    unsigned GetMissionCount() const { return (unsigned)m_missions.size(); }  // @0x57e614
    Mission *GetMission(unsigned i) const { return m_missions[i]; }           // @0x57e628
    // the one set the missions go on in when the others are done
    bool IsInfiniteMissionLooper() const { return m_isInfiniteMissionLooper; }
    static const jet::String k_tmplName;
private:
    std::vector<Mission*> m_missions;  // 0x20
    bool m_isInfiniteMissionLooper;    // 0x2c
};

#endif
