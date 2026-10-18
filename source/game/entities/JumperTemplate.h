// source/game/entities/JumperTemplate.h
#ifndef SOURCE_GAME_ENTITIES_JUMPERTEMPLATE_H
#define SOURCE_GAME_ENTITIES_JUMPERTEMPLATE_H

#include "Object.h"
#include "SynchronizedObject.h"

class JumpDefinition;

// A jumper: the minion that runs onto it is thrown up along a jump definition at a speed (the data's "JumperTemplate", type 0x11; 0x18C bytes
// in the .so). The contact is CollidesWith (@0x56f2b0). Not ported yet: GameLevel::NotifyIncreasedSpeed (@0x53ebb4, the speed-up post effect
// through EnableSpeedUpFx @0x53eb38 and EnableRadialBlur @0x518104; the GameLevel fields +0x644..+0x668 and the radial blur material are not
// decoded), which the contact calls before the jump.
class JumperTemplate : public Object, public SynchronizedObject {
public:
    explicit JumperTemplate(const clara::Template*);
    virtual ~JumperTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual void SetSpawnLane(int lane) { SynchronizedObject::SetSpawnLane((LaneIndex)lane); }
    virtual void OnMinionRevive();
    JumpDefinition *GetJumpDefinition() const { return m_jumpDefinition; }
    int GetEngineEvent() const { return m_engineEvent; }
    float GetJumpSpeed() const { return m_jumpSpeed; }
    float GetJumpFrontMinSpeed() const { return m_jumpFrontMinSpeed; }
    unsigned GetJumpFrontMinSpeedTime() const { return m_jumpFrontMinSpeedTime; }
    bool IsJumpDone() const { return m_jumpDone; }
    void SetJumpDone(bool b) { m_jumpDone = b; }
    static jet::String k_tmplName;
    static const jet::String k_jumpDefininitionParam, k_jumpSpeedParam, k_jumpFrontMinSpeedParam, k_jumpFrontMinSpeedTimeParam, k_engineEventParam;
private:
    JumpDefinition *m_jumpDefinition;  // 0x150
    bool m_jumpDone;                   // 0x154
    int m_engineEvent;                 // 0x158
    float m_jumpSpeed;                 // 0x15C
    float m_jumpFrontMinSpeed;         // 0x160
    unsigned m_jumpFrontMinSpeedTime;  // 0x164
};

#endif
