// source/game/entities/LevelDef.h
#ifndef SOURCE_GAME_ENTITIES_LEVELDEF_H
#define SOURCE_GAME_ENTITIES_LEVELDEF_H

#include "jsingleton.h"
#include "GameEntity.h"
#include "movie.h"

class ScriptData;
class Minion;
class CameraDirectEntity;
class CameraEntity;

// The level's description: the Lua functions of its scripts and the menu defaults. Entry points the game
// calls on state changes: OnInitMainMenu, OnExitMainMenu, OnInitPreGamePlay, OnInitGamePlay, DefineLevel, ...
class LevelDef : public GameEntity, public Singleton<LevelDef> {  // sizeof 0xA0
public:
    explicit LevelDef(const clara::Template*);
    virtual ~LevelDef();
    virtual void Init();
    void OnInitMainMenu();
    void OnExitMainMenu();
    void OnInitPreGamePlay();
    void OnInitGamePlay();
    void OnStartGamePlay();
    void OnInitShopMenu();
    void DefineLevel();
    void MovieLevel_OnStartMovie();
    clara::Movie *MovieLevel_GetPlayedMovie();
    clara::Entity *GetMainMenuMinion() { return m_mainMenuMinion; }
    CameraEntity *GetUIDefaultCamera3D() { return m_UIDefaultCamera3D; }
    clara::Entity *GetUIDefaultLight3D() { return m_UIDefaultLight3D; }
    clara::Movie *GetMovieFromParam(const jet::String&);
    float GetI3DMinionCostumeCamDistance();
    float GetI3DShopBigMinionCostumeCamDistance();
    float GetI3DShopSmallMinionCostumeCamDistance();
    static jet::String k_tmplName;
private:
    ScriptData *m_scriptData;  // 0x90
    clara::Entity *m_mainMenuMinion;  // 0x94
    CameraEntity *m_UIDefaultCamera3D;  // 0x98
    clara::Entity *m_UIDefaultLight3D;  // 0x9C
};

#endif
