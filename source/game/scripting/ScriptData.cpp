// source/game/scripting/ScriptData.cpp
#include "ScriptData.h"
#include "game/entities/GameEntity.h"
#include "param.h"
#include "type.h"
#include <cstdio>
#include <cstdlib>

ScriptData::ScriptData(LuaVM *vm) : m_owner(0), m_vm(vm) {}
ScriptData::~ScriptData() {}

// @0x3a92dc: "function <prefix>_<param>(<args>)<script>\nend" for each script parameter with a body
void ScriptData::Init(GameEntity *owner, const std::map<jet::String, jet::String> &args) {
    m_owner = owner;
    if (!owner || owner->GetParamCount() == 0) return;
    jet::String prefix = owner->GetScriptPrefix();
    for (unsigned i = 0; i != owner->GetParamCount(); i++) {
        clara::Param *p = owner->GetParam(i);
        if (p->GetType()->GetValueType() != clara::Type::SCRIPT || p->GetComponentCount() == 0) continue;
        const std::string &script = p->GetAsScript(0);
        if (script.empty()) continue;
        jet::String fnArgs;
        std::map<jet::String, jet::String>::const_iterator a = args.find(p->GetName());
        if (a != args.end()) fnArgs = a->second;
        std::string name = std::string(prefix.c_str()) + "_" + p->GetName().c_str();
        std::string code = std::string("function ") + name + "(" + fnArgs.c_str() + ")" + script + "\nend";
        if (getenv("JET_DUMPLUA")) fprintf(stderr, "LUA %s\n", code.c_str());
        m_vm->LoadFromString(code);
        m_funcMap[p->GetName()] = m_vm->GetFuncRef(jet::String(name.c_str()));
    }
}

bool ScriptData::IsFunctionRunning(const jet::String &name) {
    if (name.empty()) return false;
    std::unordered_map<jet::String, int>::iterator it = m_funcMap.find(name);
    return it != m_funcMap.end() && it->second >= 0 && m_vm->IsFunctionRunning(it->second);
}

// @0x3a9758: the function runs as a script thread of the owner
bool ScriptData::CallScriptFunction(const jet::String &name) {
    return CallScriptFunction(name, std::vector<ScriptParam>());
}

bool ScriptData::CallScriptFunction(const jet::String &name, const std::vector<ScriptParam> &params) {
    if (name.empty()) return false;
    std::unordered_map<jet::String, int>::iterator it = m_funcMap.find(name);
    if (it == m_funcMap.end() || it->second < 0) return false;
    m_vm->SetThisPointer(m_owner);
    m_vm->StartFunction(it->second, params, name);
    return true;
}

bool ScriptData::HasScriptFunction(const jet::String &name) {
    if (name.empty()) return false;
    std::unordered_map<jet::String, int>::iterator it = m_funcMap.find(name);
    return it != m_funcMap.end() && it->second >= 0;
}

bool ScriptData::IsRunning() { return m_vm->HasRunningThreads(m_owner); }
bool ScriptData::StopAllThreads() { return m_vm->StopThreads(m_owner, -1); }

bool ScriptData::StopThread(const jet::String &name) {
    std::unordered_map<jet::String, int>::iterator it = m_funcMap.find(name);
    return it != m_funcMap.end() && it->second >= 0 && m_vm->StopThreads(m_owner, it->second);
}
