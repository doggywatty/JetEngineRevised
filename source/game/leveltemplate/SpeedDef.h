// source/game/leveltemplate/SpeedDef.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_SPEEDDEF_H
#define SOURCE_GAME_LEVELTEMPLATE_SPEEDDEF_H

// A speed set by a level description's Speed(x) function
struct SpeedDef {  // sizeof 0x8
    bool isSet;    // 0x0
    float speed;   // 0x4
    SpeedDef() : isSet(false), speed(0) {}
};

#endif
