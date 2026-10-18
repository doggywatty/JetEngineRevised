// framework/include/luavm.h
#ifndef FRAMEWORK_INCLUDE_LUAVM_H
#define FRAMEWORK_INCLUDE_LUAVM_H

#include <string>
#include <vector>
#include "core/pstring.h"
#include "scriptparam.h"
#include "stream/istream.h"

extern "C" {
#include "lua.h"
}

class LuaVM;

// One running coroutine of a script function
class LuaThread {  // sizeof 0x28
public:
    LuaThread();
    ~LuaThread();
    int Start(int funcRef, const std::vector<ScriptParam>&, const jet::String &debugName);
    void Update(unsigned dt);
    int Consume();
    void Stop();
    void *GetThisPtr() { return m_this; }
    void *GetOtherPtr() { return m_other; }
    void *GetTargetPtr() { return m_target; }
    bool IsRunning() const { return m_thread != 0; }
    int GetFuncRef() const { return m_funcRef; }
    void *GetOwner() const { return m_this; }
protected:
    friend class LuaVM;
    int m_delay;  // 0x0: ms to wait before the next resume
    lua_State *m_thread;  // 0x4
    bool m_isRunning;  // 0x8: inside lua_resume
    LuaVM *m_vm;  // 0xC
    void *m_this;  // 0x10
    void *m_other;  // 0x14
    void *m_target;  // 0x18
    int m_funcRef;  // 0x1C
    int m_threadRef;  // 0x20
    int m_consumedTime;  // 0x24
};

// A Lua 5.1 state with a pool of 50 script threads that the game updates in lock step
class LuaVM {  // sizeof 0x2C
public:
    enum { k_maxThreads = 50 };
    LuaVM();
    ~LuaVM();
    bool LoadFromFile(const jet::String&);
    int LoadFromBuffer(const std::string&, const jet::String &name, bool run);
    int LoadFromBuffer(const std::vector<char>&, const jet::String &name, bool run);
    bool LoadFromString(const std::string&);
    int Export(jet::stream::IStreamW*);
    void Serialize(jet::stream::IStreamW&);
    void Deserialize(jet::stream::IStream&);
    void Reset();
    bool HasRunningThreads(void *owner);
    void StopThreads();
    bool StopThreads(void *owner, int funcRef);
    bool IsFunctionRunning(int funcRef);
    int Start();
    int CallFunction(const jet::String&, const std::vector<ScriptParam>&);
    int StartFunction(int funcRef, const std::vector<ScriptParam>&, const jet::String &debugName);
    int GetFuncRef(const jet::String&);
    void SetThisPointer(void *p) { m_this = p; }
    void SetOtherPointer(void *p) { m_other = p; }
    void SetTargetPointer(void *p) { m_target = p; }
    void SetGlobal(const char*, double);
    void SetGlobal(const char*, float);
    void SetGlobal(const char*, int);
    void SetGlobal(const char*, bool);
    void SetGlobal(const char*, const char*);
    void SetGlobal(const char*, const jet::String&);
    void SetGlobal(const char*, void*);
    void ClearGlobals();
    void GarbageCollect();
    void Update(unsigned dt);
    int ConsumeThreads();
    void SetSavingThread(lua_State *l) { m_savingThread = l; }
    void RegisterCFunction(const jet::String&, int (*)(lua_State*));
    jet::String GetErrorMsg();
    lua_State *GetLuaState() { return m_state; }
    LuaThread *GetCurrentThread() { return m_currentThreads.empty() ? 0 : m_currentThreads.back(); }

    static bool s_isConsumingThreads;
    static bool s_stopConsumingMe;
    std::vector<LuaThread*> m_currentThreads;  // 0x0: the stack of threads being resumed
private:
    friend class LuaThread;
    static int funcPanic(lua_State*);
    void SerializeTable(jet::stream::IStreamW&);
    void SerializeVal(jet::stream::IStreamW&, int index);
    void DeserializeVal(jet::stream::IStream&);
    bool DeserializeTable(jet::stream::IStream&);
    void CreateThreads();
    lua_State *m_state;  // 0xC
    LuaThread *m_threads;  // 0x10
    lua_State *m_savingThread;  // 0x14
    void *m_this;  // 0x18
    void *m_other;  // 0x1C
    void *m_target;  // 0x20
};

#endif
