// source/game/gamemanagers/BappleMgr.cpp
#include "BappleMgr.h"

// @0x3ae0ec, @0x3ae15c
static int SpotOf(int id) {
    static const signed char k_spots[6] = { 0, 1, 2, 3, 1, 4 };
    return (id >= 0 && id < 6) ? k_spots[id] : -1;
}
int LocationToBappleSpot(int location) { return SpotOf(location); }
int EdgeConditionToBappleSpot(int condition) { return SpotOf(condition); }

// @0x4c2644
bool BappleMgr::AreActiveObjectivesInSpot(int spot) const {
    for (const BappleObjective *o : m_activeObjectives)
        if (o->GetSpot() == spot && !o->IsCompleted()) return true;
    return false;
}

// @0x4c2f40: the objectives of the spot start over
void BappleMgr::OnEnterSpot(int) {}
