// source/game/entities/Launcher.cpp
#include "Launcher.h"
#include "BonusSet.h"
#include "Bonus.h"
#include "GameEntityCreator.h"
#include "InteractiveAnimatedPusher.h"
#include "gameitem/GameItemType.h"
#include "../GameLevel.h"
#include "param.h"
#include "project.h"

jet::String Launcher::k_tmplName("Launcher");
const float Launcher::k_hideLauncherOffsetZ = 1000.0f;
const jet::String Launcher::k_interactiveAnimatedPusherParam("interactiveAnimatedPusher"),
    Launcher::k_interactiveAnimatedPusherUpgradesParam("interactiveAnimatedPusherUpgrades");

// clara::Entity::GetLocalPosition (@0x517f7c): the offset to the link point when the entity is linked, else its position
static const math::vec3<float> &LocalPosition(const clara::Entity *e) { return e->GetLinkData() ? e->GetLinkData()->position : e->GetPosition(); }
// clara::Entity (@0x517f90): the rotation the same way
static const math::quat<float> &LocalRotation(const clara::Entity *e) { return e->GetLinkData() ? e->GetLinkData()->rotation : e->GetRotation(); }

// @0x570b08
Launcher::Launcher(const clara::Template *t)
    : GameEntity(t), m_defaultPusher(0), m_upgradePusher(0), m_spawnLane((LaneIndex)0), m_usesUpgradedPusher(false) {}

// @0x570b44: the level forgets the launcher when it goes
Launcher::~Launcher() {
    if (GameLevel *gl = GameLevel::GetInstance())
        if (gl->m_launcher == this) gl->m_launcher = 0;
}

// @0x570bc0
void Launcher::Init() {
    if (IsLibraryEntity()) return;
    GameEntity::Init();
}

// @0x570bd0: the launcher updates, and is not drawn (it is only a place)
void Launcher::PostInit() {
    if (IsLibraryEntity()) return;
    GameEntity::PostInit();
    RegisterForUpdate(true);
    RegisterForRender(false);
}

// @0x570c08: the pusher is a copy of the path's template, given the launcher's scale and linked where the launcher is linked. It is placed
// at the launcher's place plus its own offset, hidden 1000 up (the z of the position), and it takes the launcher's spawn lane
InteractiveAnimatedPusher *Launcher::CreatePusher(const clara::Path &path) {
    GameEntity *tmpl = dynamic_cast<GameEntity*>(clara::Project::GetInstance()->FindEntityByPath(path));
    GameEntityCreator creator;
    GameEntity *p = dynamic_cast<GameEntity*>(creator.CreateEntity(tmpl->GetTemplate()));
    p->ConstructFrom(tmpl);
    const math::vec3<float> ownPosition = p->GetPosition();
    const math::quat<float> ownRotation = p->GetRotation();
    p->SetTransform(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1), GetScale());
    p->LinkTo(GetLinkee(), GetLinkPointName(), 0);
    const math::vec3<float> pos = (LocalPosition(this) + ownPosition) + math::vec3<float>(0, 0, k_hideLauncherOffsetZ);
    const math::quat<float> rot = LocalRotation(this) * ownRotation;
    p->SetTransform(pos, rot, GetScale());
    p->SetPersistenceType(GameEntity::PersistenceTypeDef::Volatile);
    p->SetSpawnLane(m_spawnLane);
    return static_cast<InteractiveAnimatedPusher*>(p);
}

// @0x570dd8
void Launcher::CreateDefaultPusher() {
    m_defaultPusher = CreatePusher(FindParamByName(k_interactiveAnimatedPusherParam)->GetAsPath(0));
    m_defaultPusher->m_ownedByLauncher = true;
}

// @0x570e20: the upgrade is the path the bonus cannon's level picks. The flag is set on the default pusher, as the .so does
void Launcher::CreateUpgradePusher() {
    const int level = BonusSet::GetInstance()->GetBonus(10, false)->GetCurrentLevel();
    m_upgradePusher = CreatePusher(FindParamByName(k_interactiveAnimatedPusherUpgradesParam)->GetAsPath(level));
    m_defaultPusher->m_ownedByLauncher = true;
}

// @0x570e90
void Launcher::InitLauncherData() {
    CreateDefaultPusher();
    CreateUpgradePusher();
    InitPushers();
}

// @0x570eb0: the default pusher is put under the launcher, 1000 down its z
void Launcher::InitPushers() {
    math::vec3<float> pos = LocalPosition(m_defaultPusher);
    pos -= math::vec3<float>(0, 0, k_hideLauncherOffsetZ);
    m_defaultPusher->SetPosition(pos);
}

// @0x570f1c: on: when the upgrade is not in use, the default pusher is released and the upgrade one goes hidden. Off: when the upgrade is in
// use, it is shown again and the default pusher goes back to idle. The launcher keeps the flag the activation gave
void Launcher::ActivatePusher(bool on) {
    if (on) {
        if (!m_usesUpgradedPusher) {
            m_defaultPusher->m_pusherState = InteractiveAnimatedPusher::Released;
            math::vec3<float> pos = LocalPosition(m_upgradePusher);
            pos -= math::vec3<float>(0, 0, k_hideLauncherOffsetZ);
            m_upgradePusher->SetPosition(pos);
        }
    } else if (m_usesUpgradedPusher) {
        math::vec3<float> pos = LocalPosition(m_upgradePusher);
        pos += math::vec3<float>(0, 0, k_hideLauncherOffsetZ);
        m_upgradePusher->SetPosition(pos);
        m_defaultPusher->SetPusherState(InteractiveAnimatedPusher::Idle);
    }
    m_usesUpgradedPusher = on;
}
