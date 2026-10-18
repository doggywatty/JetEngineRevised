// source/game/scripting/ScriptData.h
#ifndef SOURCE_GAME_SCRIPTING_SCRIPTDATA_H
#define SOURCE_GAME_SCRIPTING_SCRIPTDATA_H

#include <map>
#include <unordered_map>
#include <vector>
#include "core/pstring.h"
#include "luavm.h"

class GameEntity;

// The Lua functions an entity owns: every script parameter of its template becomes a global function
class ScriptData {  // sizeof 0x20
public:
    ScriptData(LuaVM*);
    ~ScriptData();
    void Init(GameEntity*, const std::map<jet::String, jet::String> &args);
    bool IsFunctionRunning(const jet::String&);
    bool IsRunning();
    bool StopAllThreads();
    bool StopThread(const jet::String&);
    bool CallScriptFunction(const jet::String&, const std::vector<ScriptParam>&);
    bool CallScriptFunction(const jet::String&);
    bool HasScriptFunction(const jet::String&);
    bool IsValid() { return m_vm != 0; }
    bool IsEmpty() { return m_funcMap.empty(); }
protected:
    GameEntity *m_owner;  // 0x0
    LuaVM *m_vm;  // 0x4
    std::unordered_map<jet::String, int> m_funcMap;  // 0x8: script parameter name -> registry reference
};

#endif
