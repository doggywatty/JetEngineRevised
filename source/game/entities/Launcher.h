// source/game/entities/Launcher.h
#ifndef SOURCE_GAME_ENTITIES_LAUNCHER_H
#define SOURCE_GAME_ENTITIES_LAUNCHER_H

#include "GameEntity.h"
#include "../gameenums.h"

class InteractiveAnimatedPusher;

// The launcher ("Launcher" template; 0xA0 bytes in the .so): the place of the level where the minion is launched by a pusher. It makes the
// pushers its data names (the default one, and with the bonus cannon's upgrade the upgrade one, which the bonus cannon's level picks),
// places them at its place hidden 1000 up, and ActivatePusher shows the one in use (GS_Gameplay::StartCheckpointLauncher).
class Launcher : public GameEntity {
public:
    explicit Launcher(const clara::Template*);
    virtual ~Launcher();
    JET_RTTI(Launcher, GameEntity)
    virtual void Init();
    virtual void PostInit();
    virtual void SetSpawnLane(int lane) { m_spawnLane = (LaneIndex)lane; }

    // @0x570c08: the pusher the path names, made as a copy of the template and placed at the launcher (linked as the launcher is)
    InteractiveAnimatedPusher *CreatePusher(const clara::Path&);
    // @0x570dd8, @0x570e20: the pushers the data names (the upgrade one by the bonus cannon's level)
    void CreateDefaultPusher();
    void CreateUpgradePusher();
    // @0x570e90: both pushers are made and placed
    void InitLauncherData();
    // @0x570eb0: the default pusher is put hidden under the launcher
    void InitPushers();
    // @0x570f1c: the pusher in use is shown (the upgrade one when the bonus cannon is upgraded, else the default one is released)
    void ActivatePusher(bool);

    static jet::String k_tmplName;
private:
    static const float k_hideLauncherOffsetZ;
    static const jet::String k_interactiveAnimatedPusherParam, k_interactiveAnimatedPusherUpgradesParam;
    InteractiveAnimatedPusher *m_defaultPusher;  // 0x8C
    InteractiveAnimatedPusher *m_upgradePusher;  // 0x90
    LaneIndex m_spawnLane;                       // 0x94
    bool m_usesUpgradedPusher;                   // 0x98
};

#endif
