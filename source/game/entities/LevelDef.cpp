// source/game/entities/LevelDef.cpp
#include "LevelDef.h"
#include "game/entities/camera/CameraEntity.h"
#include "game/scripting/ScriptData.h"
#include "param.h"
#include "project.h"
#include "game/GameLevel.h"
#include "game/entities/Minion.h"

jet::String LevelDef::k_tmplName("LevelDef");

// @0x5474a4
LevelDef::LevelDef(const clara::Template *t) : GameEntity(t), m_scriptData(0), m_mainMenuMinion(0), m_UIDefaultCamera3D(0), m_UIDefaultLight3D(0) {}

// @0x547500
LevelDef::~LevelDef() { delete m_scriptData; }

// @0x547d14: the camera distance of the 3D costume previews
float LevelDef::GetI3DMinionCostumeCamDistance() {
    float v = 0;
    GetParam(jet::String("I3DMinionCostumeCamDistance"), v);
    return v;
}

// @0x547d68
float LevelDef::GetI3DShopBigMinionCostumeCamDistance() {
    float v = 0;
    GetParam(jet::String("I3DShopBigMinionCostumeCamDistance"), v);
    return v;
}

// @0x547dbc
float LevelDef::GetI3DShopSmallMinionCostumeCamDistance() {
    float v = 0;
    GetParam(jet::String("I3DShopSmallMinionCostumeCamDistance"), v);
    return v;
}

// @0x547590
void LevelDef::Init() {
    GameEntity::Init();
    GameLevel *gl = GameLevel::GetInstance();
    gl->SetLevelDef(this);
    m_scriptData = new ScriptData(gl->GetLuaVM());
    m_scriptData->Init(this, std::map<jet::String, jet::String>());
    bool movie = false;
    GetParam(jet::String("IsMovieLevel"), movie);
    auto find = [&](const char *name) {
        clara::Path path;
        if (const clara::Param *p = FindParamByName(jet::String(name))) path = p->GetAsPath(0);
        return dynamic_cast<clara::Entity*>(clara::Project::GetInstance()->FindEntityByPath(path));
    };
    if (!movie) {
        m_mainMenuMinion = find("MainMenuMinion");
        m_UIDefaultCamera3D = dynamic_cast<CameraEntity*>(find("UIDefaultCamera3D"));
        m_UIDefaultLight3D = find("UIDefaultLight3D");
    }
    bool fog = false;
    GetParam(jet::String("FogEnabled"), fog);
    if (fog) {
        float start = 0, end = 0;
        math::vec4<float> col(0, 0, 0, 0);
        GetParam(jet::String("FogStartDepth"), start);
        GetParam(jet::String("FogEndDepth"), end);
        gl->SetFogDepth(start, end, 0);
        GetParam(jet::String("FogColor"), col);
        gl->SetFogColor(jet::video::Color(col.x, col.y, col.z, col.w), 0);
    }
    gl->SetFog(fog, 0);
}

#define LD_CALL(fn) void LevelDef::fn() { m_scriptData->CallScriptFunction(jet::String(#fn)); }
LD_CALL(OnInitMainMenu)
LD_CALL(OnExitMainMenu)
LD_CALL(OnInitPreGamePlay)
LD_CALL(OnStartGamePlay)
LD_CALL(OnInitShopMenu)
LD_CALL(DefineLevel)
LD_CALL(MovieLevel_OnStartMovie)

// @0x5aef28: the script, then the minion starts the run
void LevelDef::OnInitGamePlay() {
    m_scriptData->CallScriptFunction(jet::String("OnInitGamePlay"));
    GameLevel::GetInstance()->GetMinion()->InitGameplay();
}

// @0x547ee4
clara::Movie *LevelDef::GetMovieFromParam(const jet::String &name) {
    const clara::Param *p = FindParamByName(name);
    return p ? clara::Project::GetInstance()->FindMovieByPath(p->GetAsPath(0)) : nullptr;
}

clara::Movie *LevelDef::MovieLevel_GetPlayedMovie() { return GetMovieFromParam(jet::String("MovieLevel_RefMovie")); }
