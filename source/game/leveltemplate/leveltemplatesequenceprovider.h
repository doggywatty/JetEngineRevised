// source/game/leveltemplate/leveltemplatesequenceprovider.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATESEQUENCEPROVIDER_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATESEQUENCEPROVIDER_H

#include "core/pstring.h"

class GameEntity;
struct SpeedDef;

// What tells the level template manager which template comes next in the run (the game level, from the graph of the location)
class LevelTemplateSequenceProvider {  // sizeof 0x4
public:
    virtual void IncSequenceNumber() = 0;
    virtual int GetNextSequenceNumber() = 0;
    virtual GameEntity *GetNextCamera() = 0;
    virtual int GetNextCameraTransitionTime() = 0;
    virtual const SpeedDef *GetNextSpeed() = 0;
    virtual const jet::String &GetNextLocation() = 0;
    virtual const jet::String &GetNextNodeLabel() = 0;
    virtual const jet::String &GetNextNodeId() = 0;
    virtual int GetNextNodeGraphId() = 0;
    virtual void OnNodeCompleted(const int &graphId, const jet::String &nodeId) = 0;
};

#endif
