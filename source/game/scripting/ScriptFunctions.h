// source/game/scripting/ScriptFunctions.h
#ifndef SOURCE_GAME_SCRIPTING_SCRIPTFUNCTIONS_H
#define SOURCE_GAME_SCRIPTING_SCRIPTFUNCTIONS_H

class LuaVM;

// The functions the level scripts call (@0x3baecc). Left for the gameplay port: LoadLevelSequence, StartGamePlay,
// SetCameraRelativeToMinionSpeed, the input switches, the tutorial / boss / rocket / powerup calls, the intro
// subtitles and the sound functions (they come with the sound manager).
void RegisterScriptFunctions(LuaVM*);

#endif
