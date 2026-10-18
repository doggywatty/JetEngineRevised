// framework/src/luavm.cpp
#include "luavm.h"
#include <cstdio>
#include <cstring>

extern "C" {
#include "lauxlib.h"
#include "lualib.h"
}

bool LuaVM::s_isConsumingThreads = false;
bool LuaVM::s_stopConsumingMe = false;

static void LogLine(const char *s) { fprintf(stderr, "%s\n", s); }

// @0x5a8284: print(...) joins tostring of every argument with tabs and logs it
static int LuaPrint(lua_State *L) {
    int n = lua_gettop(L);
    lua_getfield(L, LUA_GLOBALSINDEX, "tostring");
    std::string line;
    for (int i = 1; i <= n; i++) {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char *s = lua_tolstring(L, -1, 0);
        if (!s) return luaL_error(L, "'tostring' must return a string to 'print'");
        if (i > 1) line += "\t";
        line += s;
        lua_settop(L, -2);
    }
    LogLine(line.c_str());
    return 0;
}

// --- LuaThread ---

// @0x5a97bc
LuaThread::LuaThread() : m_delay(0), m_thread(0), m_isRunning(false), m_vm(0), m_this(0), m_other(0), m_target(0), m_funcRef(0), m_threadRef(-1), m_consumedTime(0) {}

// @0x5a97d8
LuaThread::~LuaThread() {
    if (m_thread) luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
}

static void PushParams(lua_State *L, const std::vector<ScriptParam> &params) {
    for (size_t i = 0; i < params.size(); i++) {
        const ScriptParam &p = params[i];
        if (p.type == ScriptParam::TINT) lua_pushinteger(L, p.i);
        else if (p.type == ScriptParam::TFLOAT) lua_pushnumber(L, p.f);
        else if (p.type == ScriptParam::TSTRING) lua_pushstring(L, p.s.c_str());
    }
}

// @0x5a98c8: a coroutine running the function of funcRef, resumed once at once
int LuaThread::Start(int funcRef, const std::vector<ScriptParam> &params, const jet::String&) {
    m_this = m_vm->m_this;
    m_other = m_vm->m_other;
    m_target = m_vm->m_target;
    m_thread = lua_newthread(m_vm->m_state);
    m_threadRef = luaL_ref(m_vm->m_state, LUA_REGISTRYINDEX);
    m_funcRef = funcRef;
    lua_settop(m_thread, 0);
    lua_rawgeti(m_thread, LUA_REGISTRYINDEX, funcRef);
    if (lua_type(m_thread, -1) == LUA_TNIL) {
        luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
        m_thread = 0;
        m_threadRef = -1;
        return 1;
    }
    PushParams(m_thread, params);
    m_vm->m_currentThreads.push_back(this);
    m_isRunning = true;
    int r = lua_resume(m_thread, (int)params.size());
    m_isRunning = false;
    m_vm->m_currentThreads.pop_back();
    if (m_thread) {
        if (r == LUA_YIELD) m_delay = lua_tointeger(m_thread, -1);
        else {
            if (r == LUA_ERRRUN) {
                LogLine(lua_tolstring(m_thread, -1, 0));
                lua_settop(m_thread, -2);
            }
            luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
            m_thread = 0;
            m_threadRef = -1;
        }
    }
    return 0;
}

// @0x5a980c: counts the wait down, resumes at zero
void LuaThread::Update(unsigned dt) {
    if (!m_thread) return;
    if (m_delay > 0) m_delay -= (int)dt;
    if (m_delay >= 1) return;
    m_vm->m_currentThreads.push_back(this);
    m_isRunning = true;
    int r = lua_resume(m_thread, 0);
    m_isRunning = false;
    m_vm->m_currentThreads.pop_back();
    if (!m_thread) return;
    if (r == LUA_YIELD) m_delay = lua_tointeger(m_thread, -1);
    else {
        luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
        m_thread = 0;
        m_threadRef = -1;
    }
}

// @0x5a9aa4: runs the thread to its end ignoring waits (a fast forward); returns the time it skipped
int LuaThread::Consume() {
    m_consumedTime = 0;
    if (!m_thread) return 0;
    m_consumedTime = m_delay;
    m_delay = 0;
    int guard = 1000;
    bool finished = false;
    do {
        if (LuaVM::s_stopConsumingMe) { finished = true; break; }
        m_vm->m_currentThreads.push_back(this);
        m_isRunning = true;
        int r = lua_resume(m_thread, 0);
        m_isRunning = false;
        m_vm->m_currentThreads.pop_back();
        if (!m_thread) { finished = true; break; }
        if (r != LUA_YIELD) {
            luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
            m_thread = 0;
            m_threadRef = -1;
            finished = true;
            break;
        }
        m_consumedTime += lua_tointeger(m_thread, -1);
    } while (--guard);
    if (!finished) m_consumedTime = 0;
    if (!LuaVM::s_stopConsumingMe && m_thread) {
        luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
        m_thread = 0;
        m_threadRef = -1;
    }
    LuaVM::s_stopConsumingMe = false;
    return m_consumedTime;
}

// @0x5a9bd4
void LuaThread::Stop() {
    if (!m_thread) return;
    luaL_unref(m_vm->m_state, LUA_REGISTRYINDEX, m_threadRef);
    m_thread = 0;
    m_threadRef = -1;
}

// --- LuaVM ---

void LuaVM::CreateThreads() {
    m_threads = new LuaThread[k_maxThreads];
    for (int i = 0; i < k_maxThreads; i++) m_threads[i].m_vm = this;
}

// @0x5a83d0
LuaVM::LuaVM() : m_state(0), m_threads(0), m_savingThread(0), m_this(0), m_other(0), m_target(0) {
    m_state = luaL_newstate();
    luaL_openlibs(m_state);
    lua_atpanic(m_state, funcPanic);
    lua_pushcclosure(m_state, LuaPrint, 0);
    lua_setfield(m_state, LUA_GLOBALSINDEX, "print");
    CreateThreads();
}

// @0x5a8514
LuaVM::~LuaVM() {
    delete[] m_threads;
    lua_close(m_state);
}

// @0x5a8634
int LuaVM::funcPanic(lua_State *L) {
    LogLine(lua_tolstring(L, -1, 0));
    lua_settop(L, -2);
    lua_getfield(L, LUA_GLOBALSINDEX, "ERROR");
    fprintf(stderr, "ERROR %d\n", (int)lua_tonumber(L, -1));
    return 0;
}

// @0x5a86c4
int LuaVM::LoadFromBuffer(const std::string &buf, const jet::String &name, bool run) {
    int r = luaL_loadbuffer(m_state, buf.empty() ? "" : buf.c_str(), buf.size(), name.c_str());
    if (r == 0 && run) {
        r = lua_pcall(m_state, 0, 0, 0);
        lua_settop(m_state, 0);
    }
    return r;
}

// @0x598748
int LuaVM::LoadFromBuffer(const std::vector<char> &buf, const jet::String &name, bool run) {
    int r = luaL_loadbuffer(m_state, buf.empty() ? "" : &buf[0], buf.size(), name.c_str());
    if (r == 0 && run) {
        r = lua_pcall(m_state, 0, 0, 0);
        lua_settop(m_state, 0);
    }
    return r;
}

static int ExportWriter(lua_State*, const void *p, size_t sz, void *ud) {
    if (ud) static_cast<jet::stream::IStreamW*>(ud)->Write(p, (unsigned)sz);
    return 0;
}

// @0x5a87cc: the bytecode of the function on the stack top
int LuaVM::Export(jet::stream::IStreamW *out) { return lua_dump(m_state, ExportWriter, out); }

// @0x5a8810
int LuaVM::Start() {
    int r = lua_pcall(m_state, 0, 0, 0);
    lua_settop(m_state, 0);
    return r;
}

// @0x598844: a synchronous call; the result is the number the function returns, else 0
int LuaVM::CallFunction(const jet::String &name, const std::vector<ScriptParam> &params) {
    lua_getfield(m_state, LUA_GLOBALSINDEX, name.c_str());
    int result = 0;
    if (lua_type(m_state, -1) == LUA_TNIL) {
        lua_settop(m_state, -2);
    } else {
        PushParams(m_state, params);
        lua_pcall(m_state, (int)params.size(), 1, 0);
        if (lua_isnumber(m_state, -1)) {
            result = (int)lua_tonumber(m_state, -1);
            lua_settop(m_state, -2);
        }
    }
    lua_settop(m_state, 0);
    return result;
}

// @0x5a8974
bool LuaVM::IsFunctionRunning(int funcRef) {
    for (int i = 0; i < k_maxThreads; i++)
        if (m_threads[i].m_thread && m_threads[i].m_funcRef == funcRef) return true;
    return false;
}

// @0x5a89b4: 1 started, 2 already running, 3 no free thread
int LuaVM::StartFunction(int funcRef, const std::vector<ScriptParam> &params, const jet::String &name) {
    int free = -1;
    for (int i = 0; i < k_maxThreads; i++) {
        if (!m_threads[i].m_thread) { if (free == -1) free = i; }
        else if (m_threads[i].m_funcRef == funcRef) return 2;
    }
    if (free == -1) {
        lua_settop(m_state, 0);
        return 3;
    }
    m_threads[free].Start(funcRef, params, name);
    lua_settop(m_state, 0);
    return 1;
}

// @0x5a8a44
jet::String LuaVM::GetErrorMsg() {
    const char *s = lua_tolstring(m_state, -1, 0);
    lua_settop(m_state, -2);
    return s ? jet::String(s) : jet::String("");
}

// @0x5a8ab0
void LuaVM::RegisterCFunction(const jet::String &name, int (*f)(lua_State*)) {
    lua_pushcclosure(m_state, f, 0);
    lua_setfield(m_state, LUA_GLOBALSINDEX, name.c_str());
}

// @0x5a8aec
void LuaVM::Update(unsigned dt) {
    for (int i = 0; i < k_maxThreads; i++) m_threads[i].Update(dt);
    lua_settop(m_state, 0);
}

// @0x5a8b28
bool LuaVM::LoadFromString(const std::string &s) {
    int r = luaL_loadstring(m_state, s.c_str());
    if (r != 0) return true;
    r = lua_pcall(m_state, 0, LUA_MULTRET, 0);
    if (r == 0) lua_settop(m_state, 0);
    return r != 0;
}

// @0x5a8cf0
bool LuaVM::LoadFromFile(const jet::String &path) {
    if (luaL_loadfile(m_state, path.c_str()) != 0) return true;
    return lua_pcall(m_state, 0, LUA_MULTRET, 0) != 0;
}

void LuaVM::SetGlobal(const char *name, void *p) { lua_pushlightuserdata(m_state, p); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }
void LuaVM::SetGlobal(const char *name, const jet::String &s) { lua_pushstring(m_state, s.c_str()); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }
void LuaVM::SetGlobal(const char *name, const char *s) { lua_pushstring(m_state, s); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }
void LuaVM::SetGlobal(const char *name, bool b) { lua_pushboolean(m_state, b); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }
void LuaVM::SetGlobal(const char *name, int v) { lua_pushinteger(m_state, v); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }
void LuaVM::SetGlobal(const char *name, float v) { lua_pushnumber(m_state, v); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }
void LuaVM::SetGlobal(const char *name, double v) { lua_pushnumber(m_state, v); lua_setfield(m_state, LUA_GLOBALSINDEX, name); }

// @0x5a8d38: the registry reference of a global function, -1 without one
int LuaVM::GetFuncRef(const jet::String &name) {
    lua_getfield(m_state, LUA_GLOBALSINDEX, name.c_str());
    if (lua_type(m_state, -1) > 0) {
        int ref = luaL_ref(m_state, LUA_REGISTRYINDEX);
        lua_settop(m_state, 0);
        return ref;
    }
    return -1;
}

// @0x5a8da0
int LuaVM::ConsumeThreads() {
    int most = 0;
    s_isConsumingThreads = true;
    for (int i = 0; i < k_maxThreads; i++) {
        int t = m_threads[i].Consume();
        if (t > most) most = t;
    }
    lua_settop(m_state, 0);
    s_isConsumingThreads = false;
    return most;
}

// @0x5a8e10
void LuaVM::StopThreads() {
    s_isConsumingThreads = true;
    for (int i = 0; i < k_maxThreads; i++) m_threads[i].Stop();
    lua_settop(m_state, 0);
    s_isConsumingThreads = false;
}

// @0x5a8e78: a fresh state with the base libraries only
void LuaVM::Reset() {
    delete[] m_threads;
    lua_close(m_state);
    m_state = luaL_newstate();
    luaopen_base(m_state);
    luaopen_table(m_state);
    luaopen_string(m_state);
    luaopen_math(m_state);
    CreateThreads();
}

// @0x5a9714 / @0x5a9754
bool LuaVM::HasRunningThreads(void *owner) {
    for (int i = 0; i < k_maxThreads; i++)
        if (m_threads[i].m_thread && m_threads[i].m_this == owner) return true;
    return false;
}

bool LuaVM::StopThreads(void *owner, int funcRef) {
    bool stopped = false;
    for (int i = 0; i < k_maxThreads; i++)
        if (m_threads[i].m_thread && m_threads[i].m_this == owner && (m_threads[i].m_funcRef == funcRef || funcRef < 0)) {
            m_threads[i].Stop();
            stopped = true;
        }
    return stopped;
}

// @0x5a9704
void LuaVM::GarbageCollect() { lua_gc(m_state, LUA_GCCOLLECT, 0); }

// @0x5a9650: forgets the string-keyed boolean and number globals
void LuaVM::ClearGlobals() {
    lua_getfield(m_state, LUA_GLOBALSINDEX, "_G");
    lua_pushnil(m_state);
    while (lua_next(m_state, -2)) {
        if (lua_type(m_state, -2) == LUA_TSTRING) {
            const char *key = lua_tolstring(m_state, -2, 0);
            int t = lua_type(m_state, -1);
            if (t == LUA_TBOOLEAN || t == LUA_TNUMBER) {
                lua_pushnil(m_state);
                lua_setfield(m_state, LUA_GLOBALSINDEX, key);
            }
        }
        lua_settop(m_state, -2);
    }
    lua_settop(m_state, 0);
}

// --- save game: the globals as {type, key, type, value} records ---

// @0x5a8f64
void LuaVM::Serialize(jet::stream::IStreamW &s) {
    lua_getfield(m_state, LUA_GLOBALSINDEX, "_G");
    SerializeTable(s);
    lua_settop(m_state, 0);
}

static bool IsLibraryName(const char *k) {
    static const char *names[] = { "_G", "math", "io", "coroutine", "string", "debug", "os", "table", "preload", "package" };
    for (const char *n : names) if (!strcmp(k, n)) return true;
    return false;
}

// @0x5a8fa8
void LuaVM::SerializeTable(jet::stream::IStreamW &s) {
    lua_pushnil(m_state);
    while (lua_next(m_state, -2)) {
        bool skip = lua_isstring(m_state, -2) && lua_type(m_state, -1) == LUA_TTABLE && IsLibraryName(lua_tolstring(m_state, -2, 0));
        if (!skip) {
            int t = lua_type(m_state, -1);
            if (t == LUA_TBOOLEAN || t == LUA_TNUMBER || t == LUA_TSTRING || t == LUA_TTABLE) {
                bool more = true;
                s << more;
                SerializeVal(s, -2);
                SerializeVal(s, -1);
            }
        }
        lua_settop(m_state, -2);
    }
    bool more = false;
    s << more;
}

// @0x5a91b0: tables nest one level only
void LuaVM::SerializeVal(jet::stream::IStreamW &s, int index) {
    static int depth = 0;
    int t = lua_type(m_state, index);
    switch (t) {
    case LUA_TBOOLEAN: {
        s << t;
        bool b = lua_toboolean(m_state, index) != 0;
        s << b;
        break;
    }
    case LUA_TNUMBER: {
        s << t;
        double d = lua_tonumber(m_state, index);
        s.Write(&d, 8);
        break;
    }
    case LUA_TSTRING:
        s << t;
        jet::WriteString(s, lua_tolstring(m_state, index, 0));
        break;
    case LUA_TTABLE:
        s << t;
        if (depth == 0) {
            depth = 1;
            SerializeTable(s);
            depth--;
        }
        break;
    default:
        break;
    }
}

// @0x5a92ec
void LuaVM::DeserializeVal(jet::stream::IStream &s) {
    int t = 0;
    s >> t;
    switch (t) {
    case LUA_TBOOLEAN: {
        bool b = false;
        s >> b;
        lua_pushboolean(m_state, b);
        break;
    }
    case LUA_TNUMBER: {
        double d = 0;
        s.Read(&d, 8);
        lua_pushnumber(m_state, d);
        break;
    }
    case LUA_TSTRING: {
        std::string str = jet::ReadCString(s);
        lua_pushstring(m_state, str.c_str());
        break;
    }
    case LUA_TTABLE:
        DeserializeTable(s);
        break;
    default:
        break;
    }
}

// @0x5a9590
bool LuaVM::DeserializeTable(jet::stream::IStream &s) {
    bool more = false;
    s >> more;
    bool any = false;
    while (more) {
        if (!any) lua_createtable(m_state, 0, 0);
        DeserializeVal(s);
        DeserializeVal(s);
        lua_rawset(m_state, -3);
        s >> more;
        any = true;
    }
    return any;
}

// @0x5a93f8
void LuaVM::Deserialize(jet::stream::IStream &s) {
    StopThreads();
    ClearGlobals();
    bool more = false;
    s >> more;
    while (more) {
        int keyType = 0;
        s >> keyType;
        std::string key = jet::ReadCString(s);
        int t = 0;
        s >> t;
        switch (t) {
        case LUA_TBOOLEAN: { bool b = false; s >> b; SetGlobal(key.c_str(), b); break; }
        case LUA_TNUMBER: { double d = 0; s.Read(&d, 8); SetGlobal(key.c_str(), d); break; }
        case LUA_TSTRING: { std::string v = jet::ReadCString(s); SetGlobal(key.c_str(), v.c_str()); break; }
        case LUA_TTABLE:
            if (DeserializeTable(s)) lua_setfield(m_state, LUA_GLOBALSINDEX, key.c_str());
            break;
        default: break;
        }
        s >> more;
    }
    lua_settop(m_state, 0);
}
