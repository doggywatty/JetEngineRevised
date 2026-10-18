// source/game/entities/GameEntityCreator.cpp
#include "GameEntityCreator.h"
#include "template.h"
#include "../GameLevel.h"
#include "Deco3d.h"
#include "GameplayDef.h"
#include "MaterialDef.h"
#include "RenderPassDef.h"
#include "LightDef.h"
#include "ShadowLightPoint.h"
#include "TouchInteract3d.h"
#include "Minion.h"
#include "LevelDef.h"
#include "LocationInfoEntity.h"
#include "LocationConfig.h"
#include "Object.h"
#include "ParticleSystemEntity.h"
#include "LensFlareEntity.h"
#include "LensFlareDef.h"
#include "TemplateData.h"
#include "TemplatePool.h"
#include "BackgroundData.h"
#include "BackgroundPool.h"
#include "PlatformInstance.h"
#include "ObstacleTemplate.h"
#include "SafeMinionPoint.h"
#include "JumperTemplate.h"
#include "SpeederTemplate.h"
#include "InteractiveAnimatedPusher.h"
#include "Launcher.h"
#include "Runner.h"
#include "LargeMinionGameplay.h"
#include "Boss.h"
#include "BossActionTrigger.h"
#include "TriggerEntity.h"
#include "BossGameplay.h"
#include "BossKiller.h"
#include "BossMacho.h"
#include "BossMeena.h"
#include "JumpDefinition.h"
#include "gameitem/Token.h"
#include "gameitem/Bapple.h"
#include "gameitem/GameItem.h"
#include "BananaTemplate.h"
#include "BananaTemplatePath.h"
#include "InteractiveObjectInstance.h"
#include "InteractiveObjectTemplate.h"
#include "PlatformTemplate.h"
#include "ChunkTemplate.h"
#include "MinionCostume.h"
#include "BonusForCostumes.h"
#include "Bonus.h"
#include "BonusSet.h"
#include "Buyable.h"
#include "Mission.h"
#include "MissionSet.h"
#include "MissionCondition.h"
#include "Achievement.h"
#include "AchievementSet.h"
#include "../online/store/Price.h"
#include "../dynamicobjects/EffectDef.h"
#include "camera/CameraDirectEntity.h"
#include "camera/CameraFollowEntity.h"
#include "camera/CameraCinematicEntity.h"

static clara::DataEntity *Create(const clara::Template *t) {
    const jet::String &n = t->GetName();
    if (n.Equals("Deco3d")) return new Deco3d(t);
    if (n.Equals(MaterialDef::k_tmplName)) return new MaterialDef(t);
    if (n.Equals(RenderPassDef::k_tmplName)) return new RenderPassDef(t);
    if (n.Equals(LightDef::k_tmplName)) return new LightDef(t);
    if (n.Equals(EffectDef::k_tmplName)) return new EffectDef(t);
    if (n.Equals(ShadowLightPoint::k_tmplName)) return new ShadowLightPoint(t);
    if (n.Equals(TouchInteract3d::k_tmplName)) return new TouchInteract3d(t);
    if (n.Equals(Minion::k_tmplName)) return new Minion(t);
    if (n.Equals(GameplayDef::k_tmplName)) return new GameplayDef(t);
    if (n.Equals(LevelDef::k_tmplName)) return new LevelDef(t);
    if (n.Equals(CameraDirectEntity::k_tmplName)) return new CameraDirectEntity(t);
    if (n.Equals(CameraCinematicEntity::k_tmplName)) return new CameraCinematicEntity(t);
    if (n.Equals(CameraFollowEntity::k_tmplName)) return new CameraFollowEntity(t);
    if (n.Equals(LocationInfoEntity::k_tmplName)) return new LocationInfoEntity(t);
    if (n.Equals(LocationConfig::k_tmplName)) return new LocationConfig(t);
    if (n.Equals(Object::k_tmplName)) return new Object(t);
    if (n.Equals(ParticleSystemEntity::k_tmplName)) return new ParticleSystemEntity(t);
    if (n.Equals(LensFlareDef::k_tmplName)) return new LensFlareDef(t);
    if (n.Equals(LensFlareEntity::k_tmplName)) return new LensFlareEntity(t);
    if (n.Equals(TemplateData::k_tmplName)) return new TemplateData(t);
    if (n.Equals(TemplatePool::k_tmplName)) return new TemplatePool(t);
    if (n.Equals(BackgroundData::k_tmplName)) return new BackgroundData(t);
    if (n.Equals(BackgroundPool::k_tmplName)) return new BackgroundPool(t);
    if (n.Equals(PlatformInstance::k_tmplName)) return new PlatformInstance(t);
    if (n.Equals(PlatformTemplate::k_tmplName)) return new PlatformTemplate(t);
    if (n.Equals(InteractiveObjectInstance::k_tmplName)) return new InteractiveObjectInstance(t);
    if (n.Equals(InteractiveObjectTemplate::k_tmplName) || n.Equals(InteractiveObjectTemplate::k_superflousInteractive_tmplName)) return new InteractiveObjectTemplate(t);
    if (n.Equals(JumperTemplate::k_tmplName)) return new JumperTemplate(t);
    if (n.Equals(SpeederTemplate::k_tmplName)) return new SpeederTemplate(t);
    if (n.Equals(InteractiveAnimatedPusher::k_tmplName)) return new InteractiveAnimatedPusher(t);
    if (n.Equals(Launcher::k_tmplName)) return new Launcher(t);
    if (n.Equals(Runner::k_tmplName)) return new Runner(t);
    if (n.Equals(LargeMinionGameplay::k_tmplName)) return new LargeMinionGameplay(t);
    if (n.Equals(Boss::k_tmplName)) return new Boss(t);
    if (n.Equals(BossMeena::k_tmplName)) return new BossMeena(t);
    if (n.Equals(BossMacho::k_tmplName)) return new BossMacho(t);
    // BossGameplay is not registered: its Update lacks the mount branch (GameLevel::ForceDismountActiveMount @0x53fc10 needs the Mount
    // subsystem, Mount.cpp is empty), so its whole behaviour is not ported (see re/TODO.md).
    if (n.Equals(BossActionTrigger::k_tmplName)) return new BossActionTrigger(t);
    if (n.Equals(TriggerEntity::k_tmplName)) return new TriggerEntity(t);
    if (n.Equals(BossKiller::k_tmplName)) return new BossKiller(t);
    if (n.Equals(JumpDefinition::k_tmplName)) return new JumpDefinition(t);
    if (n.Equals(Token::k_tmplName)) return new Token(t);
    if (n.Equals(Bapple::k_tmplName)) return new Bapple(t);
    if (n.Equals(GameItem::k_tmplName)) return new GameItem(t);
    if (n.Equals(SafeMinionPoint::k_tmplName)) return new SafeMinionPoint(t);
    if (n.Equals(ObstacleTemplate::k_tmplName)) return new ObstacleTemplate(t);
    if (n.Equals(BananaTemplate::k_tmplName)) return new BananaTemplate(t);
    if (n.Equals(BananaTemplatePath::k_tmplName)) return new BananaTemplatePath(t);
    if (n.Equals(ChunkTemplate::k_tmplName)) return new ChunkTemplate(t);
    if (n.Equals(MinionCostume::k_tmplName)) return new MinionCostume(t);
    if (n.Equals(Price::k_tmplName)) return new Price(t);
    if (n.Equals(BonusForCostumes::k_tmplName)) return new BonusForCostumes(t);
    if (n.Equals(Bonus::k_tmplName)) return new Bonus(t);
    if (n.Equals(BonusSet::k_tmplName)) return new BonusSet(t);
    if (n.Equals(Buyable::k_tmplName)) return new Buyable(t);
    if (n.Equals(Mission::k_tmplName)) return new Mission(t);
    if (n.Equals(MissionSet::k_tmplName)) return new MissionSet(t);
    if (n.Equals(MissionCondition::k_tmplName)) return new MissionCondition(t);
    if (n.Equals(Achievement::k_tmplName)) return new Achievement(t);
    if (n.Equals(AchievementSet::k_tmplName)) return new AchievementSet(t);
    return new clara::Entity(t);
}

// @0x53954c: the game's class for the template, which the level then notes
clara::DataEntity *GameEntityCreator::CreateEntity(const clara::Template *t) const {
    clara::DataEntity *e = Create(t);
    if (GameLevel *l = GameLevel::GetInstance()) l->EntityCreated(e);
    return e;
}
