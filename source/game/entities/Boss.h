// source/game/entities/Boss.h
#ifndef SOURCE_GAME_ENTITIES_BOSS_H
#define SOURCE_GAME_ENTITIES_BOSS_H

#include <vector>
#include "Actor.h"
#include "Pacesetter.h"

class ObstacleTemplate;
class LevelTemplateInstance;
class Puppet;

// A boss the minion is chased by along the lanes: it follows the minion's pacesetter at its offset, takes the hits of the obstacles the minion
// meets (its health), recovers, and leaves once it is beaten. The state machine of its actor and the minion are told each change by engine events.
// 0x228 bytes in the .so (the decoded layout: the offset at 0x1D0, the sound at 0x1DC, the state at 0x1E0, the pacesetter cache at 0x1E4,
// the dying time at 0x1F4/0x1F8, the health at 0x1FC/0x200, the restore time at 0x204/0x208, the win engine event at 0x20C).
class Boss : public Actor {
public:
    // the states the boss goes through (SetBehaviorState): hidden, active (it follows the minion), defeated (its health is gone),
    // dying (it counts down to leave) and exit (it is out of the scene)
    enum BossState { BossState_Hidden = 0, BossState_Active = 1, BossState_Defeated = 2, BossState_Dying = 3, BossState_Exit = 4 };

    explicit Boss(const clara::Template*);
    virtual ~Boss();
    virtual void Update();
    virtual void Render();
    virtual void PostInit();
    virtual bool CanSpawnNewEntity(LevelTemplateInstance*) const;
    void EnableBossBehavior(bool);
    // the bit of this boss in the player's boss tutorials (BossGameplay::IsTutorialCompleted reads it)
    int GetTutorialBit() const { return m_tutorialBit; }
    void SetTutorialBit(int b) { m_tutorialBit = b; }
    void DisableBossBehavior(bool);
    virtual bool IsOutOfScene() const;
    void IncreaseHealth(float);
    virtual void StartAttack();
    virtual void OnShoot();
    virtual bool CanShoot() const;
    virtual math::vec3<float> GetBombShootPosition() const;
    virtual math::vec3<float> GetShootPosition() const;
    virtual const jet::String &GetBossSoundLabel() const;
    virtual int GetStartMinionEvent() const;
    virtual int GetEndMinionEvent() const;
    virtual int GetBossStartDyingMinionEvent() const;
    virtual int GetBossExitMinionEvent() const;
    virtual void OnMinionHittedByBossObstacle(ObstacleTemplate*);
    // @0x51bb84: the puppet that moves with the boss; the boss's state machine is linked to the puppet's
    void SetPuppet(Puppet*);
    Puppet *GetPuppet() const { return m_puppet; }
    // the state the boss is in (BossState_Active: it follows the minion and can be hit by the obstacles it sends)
    BossState GetBehaviorState() const { return m_behaviorState; }
    static jet::String k_tmplName;
protected:
    virtual void UpdatePersecution();
    virtual void SM_OnChangeState(ActorState*, ActorState*);
    virtual void OnAppear();
    virtual void OnDie();
    // the place the boss keeps while it chases the minion: the minion's lane point plus the offset turned by the minion's rotation
    math::vec3<float> GetPersecutionPosition() const;
    // @0x5a67e4 (BossMacho::UpdatePosition) and @0x5a7344 (BossMeena::UpdatePosition): the boss is on the lanes of the minion's pacesetter. Its
    // lane point is the lane's pacesetter point (the lane is the lane offset's whole part, moved by the offset's fraction toward the next lane),
    // and its place is that point plus the pacesetter's rotation of the offset (the path offset, back along the pacesetter's front).
    void UpdateLanePosition(int lane, float laneOffset, float pathOffset);
    static const jet::String k_bossSoundStateOutro;
private:
    void SetBehaviorState(Boss::BossState);
protected:
    int m_tutorialBit;                   // 0x1CC: the bit of this boss in the player's boss tutorials (0 Boss, 1 BossMeena, 2 BossMacho; read by BossGameplay::IsTutorialCompleted @0x5a6094)
    math::vec3<float> m_positionOffset;  // 0x1D0
    int m_bossSoundHandle;               // 0x1DC: the boss's sound, -1 when none
    Boss::BossState m_behaviorState;     // 0x1E0
    std::vector<Pacesetter::PacestterCacheData> m_cache;  // 0x1E4: where the pacesetter was found last
    int m_dyingTimeLeft;                 // 0x1F4: ms left before the boss leaves while dying
    int m_dyingTime;                     // 0x1F8: the whole dying time (ms)
    float m_health;                      // 0x1FC
    float m_maxHealth;                   // 0x200
    int m_timeToRestore;                 // 0x204: ms the boss takes to recover its health
    int m_timeToRestoreCount;            // 0x208: ms left before the health comes back
    int m_winEngineEvent;                // 0x20C: the engine event of the minion when the boss is beaten
    Puppet *m_puppet;                    // 0x1F0
    static const jet::String k_posOffset;
    static const jet::String k_health;
    static const jet::String k_timeToResTore;
    static const jet::String k_winEngineEvent;
    static const jet::String k_vectorSoundLabel;
    static const jet::String k_bossSoundStateIntroLoop;
};

#endif
