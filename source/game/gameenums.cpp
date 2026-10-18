// source/game/gameenums.cpp
#include "gameenums.h"

static const char *k_locationNames[ELocationDef::COUNT] = { "laboRoom", "residential", "machoLab", "minionsBeach", "resiNight", "mall" };

const jet::String &LocationIdToString(const ELocation &l) {
    static const jet::String names[ELocationDef::COUNT] = { jet::String(k_locationNames[0]), jet::String(k_locationNames[1]), jet::String(k_locationNames[2]),
                                                            jet::String(k_locationNames[3]), jet::String(k_locationNames[4]), jet::String(k_locationNames[5]) };
    return l.underlying() < ELocationDef::COUNT ? names[l.underlying()] : jet::String::null;
}

int LocationStringToId(const jet::String &s) {
    for (int i = 0; i < ELocationDef::COUNT; i++)
        if (s.Equals(k_locationNames[i])) return i;
    return -1;
}
