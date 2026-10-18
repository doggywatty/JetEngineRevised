// source/game/entities/BossGameplay.h
#ifndef SOURCE_GAME_ENTITIES_BOSSGAMEPLAY_H
#define SOURCE_GAME_ENTITIES_BOSSGAMEPLAY_H

#include <vector>
#include "GameEntity.h"
#include "Pacesetter.h"
#include "../leveltemplate/SpeedDef.h"
#include "../leveltemplate/leveltemplatesequenceprovider.h"

class Boss;
class Puppet;
class BackgroundData;
class CameraController;
class CameraEntity;
class LevelTemplateMgr;

// The boss gameplay of a location: the level sequence the run takes while the boss is on, the boss and its puppet (made from the data's boss
// reference), the pacesetter the minion follows in this gameplay, and the states it goes through (waiting for the minion to reach its plane,
// running, the final flash, done). Layout decoded from the .so: 0x108 bytes (state at 0x90, boss at 0x98, puppet at 0x9C, the pacesetter at 0xDC).
// The boss's kind is dispatched by CreateBoss (BossMeena, BossMacho, or Boss). This class is not registered in GameEntityCreator yet: the
// mount dismount of Update (GameLevel::ForceDismountActiveMount) is not ported.
class BossGameplay : public GameEntity, public LevelTemplateSequenceProvider {
public:
    // the states of the gameplay: waiting for the minion, running, the final flash is due, and done
    enum BossGameplayState { BossGameplayState_Waiting = 0, BossGameplayState_Running = 1, BossGameplayState_Final = 2, BossGameplayState_Done = 3 };
    BossGameplay(const clara::Template*);
    virtual ~BossGameplay();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    // @0x5a5814: the pacesetter the boss gameplay runs the minion on (made once, from the Pacesetter template)
    void CreatePacesetter();
    // @0x5a5698: the boss of the data's reference, with its linked puppet
    void CreateBoss();
    // @0x5a5ad8: the boss gameplay begins: the level template manager of the gameplay, the boss and its pacesetter, the minion moves to the
    // gameplay's pacesetter, the camera of the gameplay, and the boss behaviour is on
    void Start();
    // @0x5a5d34: the boss gameplay ends: the minion gets its pacesetter back, the boss and its puppet are gone, and the level is given back
    void End();
    // @0x5a5ecc
    void Reset();
    // @0x5a6014: the boss leaves (its state changes to the exit state the gameplay's killer names, and its return offset is taken)
    void KillBoss(const jet::String &exitState, const jet::String &defeatedExitState);
    void SetState(BossGameplayState s) { m_state = s; }
    LevelTemplateMgr *GetLevelTemplateMgr() const { return m_levelTemplateMgr; }
    Boss *GetBoss() const { return m_boss; }
    Puppet *GetBossPuppet() const { return m_bossPuppet; }
    // @0x5a6094: the player has completed the tutorial of this boss
    bool IsTutorialCompleted() const;
    // @0x5a60d0: the boss's bit is set in the player's boss tutorials
    void CompleteTutorial();
    // @0x5a6234 / @0x5a6274: the tutorial of the boss begins (the input is reset and the level's tutorial flag cleared) and ends
    void BeginTutorial();
    void EndTutorial();

    // LevelTemplateSequenceProvider
    virtual void IncSequenceNumber();
    virtual int GetNextSequenceNumber();
    virtual GameEntity *GetNextCamera();
    virtual int GetNextCameraTransitionTime();
    virtual const SpeedDef *GetNextSpeed();
    virtual const jet::String &GetNextLocation();
    virtual const jet::String &GetNextNodeLabel();
    virtual const jet::String &GetNextNodeId();
    virtual int GetNextNodeGraphId();
    virtual void OnNodeCompleted(const int &graphId, const jet::String &nodeId);
    static jet::String k_tmplName;
private:
    // the boss gameplay's update (@0x5a58c4): waits for the minion to reach the boss plane, then starts; the final flash when it is due
    void UpdateWaiting();
    static const jet::String k_bossRefParam;
    static const jet::String k_levelTemplateListParam;
    static const jet::String k_bossCameraParam;
    static const jet::String k_cameraTransitionTimeParam;
    static const jet::String k_environmentPoolParam;
    static const jet::String k_postGameplaylnvulnerabilityTimeParam;
    static const jet::String k_minionSpeedParam;
    static const jet::String k_returnLaneOffsetParam;
    static const jet::String k_hasTutorialParam;
    static const jet::String k_tutorialLevelTemplateParam;
    static const jet::String k_emptyStr;

    BossGameplayState m_state;                               // 0x90
    clara::DataEntity *m_bossRef;                            // 0x94: the data's boss reference
    Boss *m_boss;                                            // 0x98
    Puppet *m_bossPuppet;                                    // 0x9C
    int m_sequenceIdx;                                       // 0xA0: the index of the sequence the run is on
    std::vector<unsigned> m_sequences;                       // 0xA4: the level sequences of the gameplay
    CameraEntity *m_cameraEntity;                            // 0xB4
    int m_cameraTransitionTime;                              // 0xB8
    float m_minionSpeed;                                     // 0xBC
    std::vector<BackgroundData*> m_backgroundArray;          // 0xC0: the environment pool's backgrounds
    float m_invulnerabilityTime;                             // 0xD0
    float m_returnLaneOffset;                                // 0xD4
    float m_winReturnOffsetInLane;                           // 0xD8
    Pacesetter *m_pacesetter;                                // 0xDC: the boss gameplay's own pacesetter
    Pacesetter *m_oldPacesetter;                             // 0xE0: the pacesetter the minion had before the gameplay
    LevelTemplateMgr *m_levelTemplateMgr;                    // 0xE4
    unsigned m_minionSavedA;                                 // 0xE8: the minion's distance base kept while the gameplay runs
    unsigned m_minionSavedB;                                 // 0xEC: and its distance run
    SpeedDef m_speedDef;                                     // 0xF0
    bool m_tutorialStarted;                                  // 0xF8
    bool m_hasTutorial;                                      // 0xF9
    int m_tutorialTemplateId;                                // 0xFC
    bool m_tutorialBegun;                                    // 0x100
};

#endif
