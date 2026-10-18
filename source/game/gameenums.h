// source/game/gameenums.h
#ifndef SOURCE_GAME_GAMEENUMS_H
#define SOURCE_GAME_GAMEENUMS_H

#include "jenum.h"
#include "core/pstring.h"

// The places the game runs in (the "environmentId" enum of the clara data)
struct ELocationDef {
    enum type { LABO_ROOM, RESIDENTIAL, MACHO_LAB, MINIONS_BEACH, RESI_NIGHT, MALL, COUNT };
};
typedef safe_enum<ELocationDef> ELocation;

// How a level template splits the run: it does not, turns left or right, or lets the player choose (a tee)
enum Bifurcation { Bifurcation_Center = 0, Bifurcation_Left = 1, Bifurcation_Right = 2, Bifurcation_Tee = 3 };

// The three lanes of a level template (the names of its Dummy_L, Dummy_C and Dummy_R)
enum LaneIndex { Lane_Left = 0, Lane_Center = 1, Lane_Right = 2, Lane_Count = 3 };

// The power ups the player can have on (their order is the one of the status array of PowerUpMgr)
struct EPowerUpDef {
    enum type { Magnet = 0, GodMode, Invincible, Shield, BananaMultiplier, DespicableMultiplier, Weapon, COUNT };
};
typedef safe_enum<EPowerUpDef> EPowerUp;

// The location's name as the data spells it, and back (-1 for an unknown name) (@0x3bde14, @0x3bdeb8)
const jet::String &LocationIdToString(const ELocation&);
int LocationStringToId(const jet::String&);

#endif
