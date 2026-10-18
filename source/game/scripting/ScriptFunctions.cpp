// source/game/scripting/ScriptFunctions.cpp
#include "ScriptFunctions.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/entities/MaterialDef.h"
#include "game/entities/Deco3d.h"
#include "game/entities/ShadowLightPoint.h"
#include "camera/cameracontroller.h"
#include "game/entities/camera/CameraEntity.h"
#include "entity.h"
#include "folder.h"
#include "group.h"
#include "luavm.h"
#include "movie.h"
#include "project.h"
#include "video/textureloader.h"
#include <cmath>

extern "C" {
#include "lauxlib.h"
}

static clara::Project *P() { return clara::Project::GetInstance(); }

// @0x3a9988: an entity from the script: a path to it, or a number: the thread's this / other / target, or an id
static clara::DataEntity *GetEntityArg(lua_State *L, int i) {
    if (lua_type(L, i) == LUA_TSTRING) {
        const char *s = lua_tostring(L, i);
        clara::Path path;
        if (s) path.Parse(s, (unsigned)strlen(s));
        return P()->FindEntityByPath(path);
    }
    int v = (int)lua_tointeger(L, i);
    LuaThread *t = GameLevel::GetInstance()->GetLuaVM()->GetCurrentThread();
    switch (v) {
    case -1: return static_cast<clara::DataEntity*>(t->GetThisPtr());
    case -3: return static_cast<clara::DataEntity*>(t->GetOtherPtr());
    case -4: return static_cast<clara::DataEntity*>(t->GetTargetPtr());
    case -2:
    case -5: return 0;
    }
    return P()->FindEntityById((unsigned)v);
}

static clara::Group *GetGroupArg(lua_State *L) {
    if (lua_type(L, 1) == LUA_TSTRING) {
        const char *s = lua_tostring(L, 1);
        return s ? P()->FindGroupByName(jet::String(s)) : 0;
    }
    return P()->FindGroupById((unsigned)lua_tonumber(L, 1));
}

static clara::Movie *GetMovieArg(lua_State *L) {
    if (lua_type(L, 1) == LUA_TSTRING) {
        const char *s = lua_tostring(L, 1);
        return s ? P()->FindMovieByName(jet::String(s)) : 0;
    }
    return 0;
}

static bool GetBoolArg(lua_State *L, int i) {
    if (lua_type(L, i) == LUA_TBOOLEAN) return lua_toboolean(L, i) != 0;
    return lua_isnumber(L, i) && lua_tointeger(L, i) != 0;
}

// the entity, or each of the group when the argument names one
template <class F>
static void ForEntities(lua_State *L, F f) {
    if (clara::DataEntity *e = GetEntityArg(L, 1)) f(e);
    else if (clara::Group *g = GetGroupArg(L))
        for (unsigned i = 0; i < g->GetEntityCount(); i++) f(g->GetEntity(i));
}

static int L_Print(lua_State *L) { lua_tolstring(L, 1, 0); return 0; }
static int L_Enable(lua_State *L) { ForEntities(L, [](clara::DataEntity *e) { e->SetEnabled(true); }); return 0; }
static int L_Disable(lua_State *L) { ForEntities(L, [](clara::DataEntity *e) { e->SetEnabled(false); }); return 0; }
static int L_Show(lua_State *L) { ForEntities(L, [](clara::DataEntity *e) { e->SetVisible(true); }); return 0; }
static int L_Hide(lua_State *L) { ForEntities(L, [](clara::DataEntity *e) { e->SetVisible(false); }); return 0; }
static int L_IsEnabled(lua_State *L) { clara::DataEntity *e = GetEntityArg(L, 1); lua_pushboolean(L, e ? e->IsEnabled() : 0); return 1; }
static int L_IsVisible(lua_State *L) { clara::DataEntity *e = GetEntityArg(L, 1); lua_pushboolean(L, e ? e->IsVisible() : 0); return 1; }

// @0x3a9da8: the wait of a script rounded up to whole frames
static int L_GetWaitTime(lua_State *L) {
    int ms = (int)lua_tointeger(L, 1);
    lua_settop(L, -2);
    int step = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    if (step == 0) step = 1;
    lua_pushinteger(L, step * (ms / step + 1));
    return 1;
}

// @0x3aa5f0: SetPostFx(material, [fade])
static int L_SetPostFx(lua_State *L) {
    MaterialDef *m = dynamic_cast<MaterialDef*>(GetEntityArg(L, 1));
    bool fade = lua_type(L, 2) == LUA_TBOOLEAN ? GetBoolArg(L, 2) : false;
    GameLevel::GetInstance()->SetCurrentPostEffect(m, fade, GameLevel::k_fadeFx_defaultTechnique, 300, 0, 300, jet::core::InterpolationDef::LINEAR, jet::core::InterpolationDef::LINEAR, 0);
    return 0;
}

static int L_NoResult(lua_State *L) { lua_tointeger(L, 1); return 0; }
static int L_Nothing(lua_State*) { return 0; }

// @0x3aa210: SetCamera(camera, blendMs)
static int L_SetCamera(lua_State *L) {
    clara::DataEntity *e = GetEntityArg(L, 1);
    unsigned ms = (unsigned)lua_tointeger(L, 2);
    GameLevel *gl = GameLevel::GetInstance();
    if (gl->IsCamera(dynamic_cast<clara::Entity*>(e)))
        if (CameraEntity *c = dynamic_cast<CameraEntity*>(e)) gl->SetCamera(c->GetCameraController(), ms);
    return 0;
}

// @0x3aa328: SetRotation(entity, x, y, z) in degrees
static int L_SetRotation(lua_State *L) {
    float x = (float)lua_tonumber(L, 2) * 0.017453292f, y = (float)lua_tonumber(L, 3) * 0.017453292f, z = (float)lua_tonumber(L, 4) * 0.017453292f;
    math::quat<float> q(0, 0, 0, 1);
    q.setFromEulerZXY(math::vec3<float>(x, y, z));
    ForEntities(L, [&](clara::DataEntity *e) { static_cast<clara::Entity*>(e)->SetRotation(q); });
    return 0;
}

// @0x3aa450, 0x3aa4e0
static int L_SetFog(lua_State *L) {
    bool on = lua_type(L, 1) == LUA_TBOOLEAN ? GetBoolArg(L, 1) : false;
    unsigned ms = 0;
    if (lua_type(L, 2) == LUA_TNUMBER) { double d = lua_tonumber(L, 2); ms = d > 0 ? (unsigned)d : 0; }
    GameLevel::GetInstance()->SetFog(on, ms);
    return 0;
}

static int L_SetFogColor(lua_State *L) {
    float r = (float)lua_tonumber(L, 1), g = (float)lua_tonumber(L, 2), b = (float)lua_tonumber(L, 3), a = (float)lua_tonumber(L, 4);
    unsigned ms = 0;
    if (lua_type(L, 5) == LUA_TNUMBER) { double d = lua_tonumber(L, 5); ms = d > 0 ? (unsigned)d : 0; }
    GameLevel::GetInstance()->SetFogColor(jet::video::Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f), ms);
    return 0;
}

// @0x3aa68c: PlayMovie(movie, [loop]) starts it from its beginning and answers its length in ms
static int L_PlayMovie(lua_State *L) {
    GameLevel::GetInstance()->RemoveEntities();
    clara::Movie *m = GetMovieArg(L);
    bool loop = lua_type(L, 2) == LUA_TBOOLEAN ? GetBoolArg(L, 2) : false;
    if (!m) return 0;
    m->SetTime(m->GetStart());
    m->SetPlaying(true);
    m->SetLooping(loop);
    lua_pushinteger(L, m->GetEnd() - m->GetStart());
    return 1;
}

// @0x3aa7c0
static int L_PauseMovie(lua_State *L) {
    if (clara::Movie *m = GetMovieArg(L)) m->SetPlaying(false);
    return 0;
}

// @0x3a9fa4: the name of the device variant of an entity, movie, group or folder
static int L_GetDeviceSpecificNodeFrom(lua_State *L) {
    Game *g = Game::GetInstance();
    clara::DataEntity *e = g->GetDeviceSpecificEntity(GetEntityArg(L, 1));
    if (e) { lua_pushstring(L, e->GetName().c_str()); return 1; }
    clara::Movie *m = g->GetDeviceSpecificMovie(GetMovieArg(L));
    if (m) { lua_pushstring(L, m->GetName().c_str()); return 1; }
    clara::Group *gr = g->GetDeviceSpecificGroup(GetGroupArg(L));
    if (gr) { lua_pushstring(L, gr->GetName().c_str()); return 1; }
    clara::Folder *f = g->GetDeviceSpecificFolder(0);
    if (f) { lua_pushstring(L, f->GetName().c_str()); return 1; }
    return 0;
}

// @0x3a9eb0: SetTimeSpeed(speed, transitionMs, holdMs)
static int L_SetTimeSpeed(lua_State *L) {
    float speed = lua_type(L, 1) == LUA_TNUMBER ? (float)lua_tonumber(L, 1) : 1.0f;
    int t = lua_type(L, 2) == LUA_TNUMBER ? (int)lua_tointeger(L, 2) : 0;
    int h = lua_type(L, 3) == LUA_TNUMBER ? (int)lua_tointeger(L, 3) : 0;
    Game::GetInstance()->SetTimeSpeed(speed, t, h);
    return 0;
}

// @0x3aaadc
static int L_SetCurvature(lua_State *L) {
    Game::GetInstance()->SetCurvatureParams(math::vec2<float>((float)lua_tonumber(L, 1), (float)lua_tonumber(L, 2)), (unsigned)lua_tointeger(L, 3));
    return 0;
}

// @0x3aa9ac: LoadTexture(name) makes sure it is in memory
static int L_LoadTexture(lua_State *L) {
    const char *s = lua_tolstring(L, 1, 0);
    jet::String name(s ? s : "");
    if (!name.Equals(jet::String::null)) jet::video::TextureLoader::GetInstance()->Load(name);
    return 0;
}

// @0x3b9e48: LoadLevelSequence(graph) starts the run of the player's location
static int L_LoadLevelSequence(lua_State *L) {
    const char *s = lua_tolstring(L, 1, 0);
    GameLevel::GetInstance()->LoadLevelSequence(jet::String(s ? s : ""));
    return 0;
}

// @0x425f54: ShowCheckpointUI(on) shows or hides the checkpoint panel of the run
static int L_ShowCheckpointUI(lua_State *L) {
    GameLevel::GetInstance()->ShowCheckpointUI(GetBoolArg(L, 1));
    return 0;
}

// ShowPerksUI(on) shows or hides the perks panel of the run (its wrapper address is not decoded yet)
static int L_ShowPerksUI(lua_State *L) {
    GameLevel::GetInstance()->ShowPerksUI(GetBoolArg(L, 1));
    return 0;
}

// @0x425f40: ResumeLevelCreation() lets the level templates go on past a pause template (the lab's trigger calls it)
static int L_ResumeLevelCreation(lua_State*) {
    GameLevel::GetInstance()->GetLevelTemplateMgr()->ResumeCreation();
    return 0;
}

// @0x425e00: StartGamePlay() begins the run
static int L_StartGamePlay(lua_State*) {
    GameLevel::GetInstance()->StartGameplay();
    return 0;
}

// @0x4261a0: SetShadowLightPoint(light, deco) takes the model of a Deco3d as what the shadow light looks at
static int L_SetShadowLightPoint(lua_State *L) {
    GetEntityArg(L, 1);
    clara::DataEntity *e = GetEntityArg(L, 2);
    Deco3d *deco = (e && e->GetTemplateName().Equals(Deco3d::k_tmplName)) ? static_cast<Deco3d*>(e) : 0;
    ShadowLightPoint::SetModelRefPoint(deco ? deco->GetModel() : 0);
    return 0;
}

void RegisterScriptFunctions(LuaVM *vm) {
    struct E { const char *name; int (*f)(lua_State*); };
    static const E k[] = {
        { "Print", L_Print }, { "Enable", L_Enable }, { "Disable", L_Disable }, { "IsEnabled", L_IsEnabled }, { "Show", L_Show }, { "Hide", L_Hide },
        { "IsVisible", L_IsVisible }, { "GetWaitTime", L_GetWaitTime }, { "SetPostFx", L_SetPostFx }, { "FadeOut", L_NoResult }, { "FadeIn", L_NoResult },
        { "StopMusic", L_Nothing }, { "StopSound", L_Nothing }, { "GetSoundDuration", L_Nothing }, { "GetMusicDuration", L_Nothing }, { "SetClip", L_Nothing },
        { "PlayClip", L_Nothing }, { "PauseClip", L_Nothing }, { "SetClipTime", L_Nothing }, { "SetClipLoop", L_Nothing }, { "OnWait", L_Nothing },
        { "SetTimeSpeed", L_SetTimeSpeed }, { "SetCamera", L_SetCamera }, { "SetRotation", L_SetRotation }, { "PlayMovie", L_PlayMovie },
        { "PauseMovie", L_PauseMovie }, { "LoadTexture", L_LoadTexture }, { "GetDeviceSpecificNodeFrom", L_GetDeviceSpecificNodeFrom },
        { "SetCurvature", L_SetCurvature }, { "SetFog", L_SetFog }, { "SetFogColor", L_SetFogColor }, { "LoadLevelSequence", L_LoadLevelSequence },
        { "SetShadowLightPoint", L_SetShadowLightPoint }, { "StartGamePlay", L_StartGamePlay }, { "ResumeLevelCreation", L_ResumeLevelCreation },
        { "ShowCheckpointUI", L_ShowCheckpointUI }, { "ShowPerksUI", L_ShowPerksUI } };
    for (const E &e : k) vm->RegisterCFunction(jet::String(e.name), e.f);
    // @0x3baf00's prelude: Wait yields the wait to the VM
    vm->LoadFromString("function Wait( time )\n\tOnWait(time)\n\tcoroutine.yield(time)\n\treturn GetWaitTime(time)\nend\n");
}
