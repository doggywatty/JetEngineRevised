// source/game/entities/LargeMinionGameplay.h
#ifndef SOURCE_GAME_ENTITIES_LARGEMINIONGAMEPLAY_H
#define SOURCE_GAME_ENTITIES_LARGEMINIONGAMEPLAY_H

#include <vector>
#include "GameEntity.h"
#include "../leveltemplate/leveltemplatesequenceprovider.h"
#include "../leveltemplate/SpeedDef.h"
#include "../gameenums.h"
#include "Runner.h"

class Pacesetter;
class CameraEntity;
class MaterialDef;
class LevelTemplateMgr;

// The large minion mode of the gameplay (the data's "LargeMinionGameplay"). It runs the level's sequences of runner waves while the minion is
// large: it spawns the runners of a wave along its own pacesetter, balances the minion's tilt against the accelerometer and ends the mode when
// the gameplay time is over. Its layout is the .so's (LargeMinionGameplay ctor @0x56f528, sizeof 0x138); the data comes from the location's
// LargeMinionConfig (LocationConfig::LoadLargeMinionConfig @0x57283c), which fills the sequences and the runners of the mode.
class LargeMinionGameplay : public GameEntity, public LevelTemplateSequenceProvider {  // sizeof 0x138
public:
    // a wave of runners: how many, in which lane (1: a random side lane), how fast they run, how long until the next wave, and where the
    // first runner and the others sit along the pacesetter's path (the .so's LargeMinionGameplayWave, 0x2C bytes)
    struct LargeMinionGameplayWave {
        math::vec2<float> numMinions;            // 0x0: the count of runners of the wave (min, max)
        int lane;                                // 0x8: the lane of the runners, 1 picks one of the sides at random
        math::vec2<float> speedPgt;              // 0xC: the speed factor of the runners (scaled by 0.01 at load)
        math::vec2<float> nextWaveTime;          // 0x14: the time until the next wave (ms)
        math::vec2<float> firstRunnerDistance;   // 0x1C: the distance of the first runner along the path
        math::vec2<float> distanceBetweenRunner; // 0x24: the distance between two runners
    };
    // a sequence of the mode: the template index it belongs to, the background it is on, and its waves (the .so's
    // LargeMinionGameplaySequence, 0x2C bytes; the waves are at 0x20)
    struct LargeMinionGameplaySequence {
        int gameplayTemplateIndex = 0;           // 0x0
        clara::Path background;                  // 0x4: the path of the background entity (its name is the location)
        std::vector<LargeMinionGameplayWave> waves;  // 0x20
    };
    // the configuration of a location's mode (the .so's LocationConfig+0x6C, 0x18 bytes): the sequences, then the runners
    struct LargeMinionGameplayConfig {
        std::vector<LargeMinionGameplaySequence> sequences;  // 0x0
        std::vector<Runner*> runners;                        // 0xC
    };

    LargeMinionGameplay(const clara::Template*);
    virtual ~LargeMinionGameplay();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void NotifyParamChanged(const jet::String&);
    void Start(const ELocation&);
    void End();
    LevelTemplateMgr *GetLevelTemplateMgr() const { return m_levelTemplateMgr; }
    MaterialDef *GetPostFxMaterial() const { return m_postFx; }
    int GetTotalPlayTime() const { return m_gameplayTime; }
    int GetBalanceState() const { return m_balanceState; }
    float GetRunnerDistanceToHitSq() const { return m_runnerDistanceToHitSq; }
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
    void AddCreatedEntity(GameEntity*);
    void RemoveCreatedEntity(GameEntity*);
    void CleanCreatedEntities();
    void OnRunnerHit(LaneIndex) {}  // @0x56fef0: empty in the .so
private:
    void UpdateTimeEnd();
    void UpdateRunnerSpawn(int ms);
    const LargeMinionGameplayConfig &FindConfigToUse(const ELocation&) const;
public:
    static const jet::String k_tmplName;
    static const jet::String k_largeMinionGameplayWave_tmplName;
    static const jet::String k_largeMinionGameplaySequence_tmplName;
    static const jet::String k_largeMinionGameplayConfig_tmplName;
private:
    unsigned m_gameplayTime;                      // 0x90: the gameplay time of the mode (ms)
    CameraEntity *m_cameraEntity;                 // 0x94: the camera of the mode
    int m_cameraTransitionTime;                   // 0x98
    int m_postGameplayInvulnerabilityTime;        // 0x9C
    const LargeMinionGameplayConfig *m_config;    // 0xA0: the configuration of the location the mode runs in
    MaterialDef *m_postFx;                        // 0xA4: the post effect of the mode
    float m_minionSpeed;                          // 0xA8
    int m_gameplayTimeCount;                      // 0xAC: the time the mode has run (ms)
    float m_tiltTarget;                           // 0xB0: the tilt the accelerometer asks for
    float m_currentTilt;                          // 0xB4: the tilt the minion has
    const LargeMinionGameplaySequence *m_currentSequence;  // 0xBC
    int m_runnerSpawnTimer;                       // 0xC0: the time until the next wave (ms)
    int m_runnerWaveIdx;                          // 0xC4: the wave of the sequence that comes next
    int m_balanceState;                           // 0xC8: 0 unbalanced to one side, 1 balanced, 2 unbalanced to the other
    jet::String m_currentLocation;                // 0xCC
    Pacesetter *m_pacesetter;                     // 0xD0: the pacesetter of the mode
    Pacesetter *m_oldPacesetter;                  // 0xD4: the minion's pacesetter before the mode
    LevelTemplateMgr *m_levelTemplateMgr;         // 0xD8
    std::vector<GameEntity*> m_createdEntities;   // 0xDC: the entities the mode created (runners, bapples)
    int m_savedDistanceBase;                      // 0xE8: the minion's distance fields before the mode (restored at the end)
    int m_savedDistanceRun;                       // 0xEC
    clara::DataEntity *m_tutorialGuiObject;       // 0xF0: the tutorial's GuiObject (its entity, the accelerometer popup is not ported)
    int m_unbalanceTime;                          // 0xF4: the time the minion may stay unbalanced (ms)
    float m_attackLaneRange;                      // 0xF8: the tilt range that attacks the lane
    float m_runnerDistanceToHitSq;                // 0xFC: the distance (squared) of a runner hit
    float m_runnerAccelerationDistance;           // 0x100
    bool m_enableDebug;                           // 0x104
    bool m_enableSliders;                         // 0x105
    jet::String m_backgroundSound;                // 0x108
    int m_backgroundSoundId;                      // 0x10C: the handle of the sound the mode plays
    static const jet::String k_paramSpecificCamera;
    static const jet::String k_paramCameraTransitionTime;
    static const jet::String k_paramPostGamplaylnerabilityTime;
    static const jet::String k_paramPostFx;
    static const jet::String k_paramUnbalanceTime;
    static const jet::String k_paramBackgroundSound;
    static const jet::String k_paramAttackLaneRange;
    static const jet::String k_paramRunnerDistanceToHit;
    static const jet::String k_paramRunnerAccelerationDistance;
    static const jet::String k_paramEnableDebug;
    static const jet::String k_paramEnableSliders;
    static const jet::String k_paramTutorialGuiObject;
    static const jet::String k_paramMinionSpeed;
    static const jet::String k_emptyStr;
};

#endif
