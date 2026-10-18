// source/game/dynamicobjects/EffectMgr.cpp
#include "EffectMgr.h"
#include "EffectDef.h"
#include "ModelPool.h"
#include <algorithm>
#include "particlemgr.h"
#include "anim/animationloader.h"
#include "camera/cameracontroller.h"
#include "camera/cameramgr.h"
#include "core/util.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "scene/modelloader.h"
#include "scene/scenemgr.h"
#include "sprites/sprite.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/entities/LightDef.h"
#include "game/gamestates/GS_Gameplay.h"
#include "game/interfaces/menus/Menu_Ingame.h"

int EffectMgr::s_lastUsedLayerId = 0;
const float EffectMgr::k_bounceMoveToScreenTime = 200.0f;

// @0x59b314: the 200 slots are free, nothing bounces on the screen
EffectMgr::EffectMgr() : m_lastAdded(0) {}

// @0x59b41c
EffectMgr::~EffectMgr() {}

// @0x59b544
EffectControl *EffectMgr::Add(int layerId, const EffectDef *def) {
    return Add(layerId, def, math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1), math::vec3<float>(1, 1, 1), false, 0, -1, true, 0) ? m_lastAdded : 0;
}

// @0x59b5cc
bool EffectMgr::Add(int layerId, const EffectDef *def, const math::vec3<float> &position, const math::quat<float> &rotation, const math::vec3<float> &scale, bool autoRemove,
                    const jet::scene::Node *node, int lifeTime, bool flashScreen, EffectControlListener *listener) {
    if (!def) return false;
    EffectControl *ec = 0;
    for (int i = 0; i < k_maxEffects && !ec; i++)
        if (!m_controls[i].active) ec = &m_controls[i];
    if (!ec) {
        m_lastAdded = 0;
        return false;
    }
    ec->layerId = layerId;
    ec->tag = 0;
    ec->model = 0;
    ec->def = def;
    ec->light = 0;
    ec->autoRemove = autoRemove;
    ec->node = node;
    ec->lifeTime = lifeTime;
    ec->lightTime = -1;
    ec->position = position;
    ec->soundHandle = -1;
    ec->listener = listener;
    if (flashScreen && def->GetUseFlashScreen()) def->FlashScreen();
    def->ShakeCamera();

    if (!def->GetModelName().empty() && jet::scene::ModelLoader::GetInstance()->CanLoad(def->GetModelName())) {
        jet::scene::Model *shadow = 0;
        ec->model = ModelPool::GetInstance()->GetModel(layerId, def->GetModelName(), def->GetShadowModelName(), &shadow);
        if (jet::scene::Model *model = ec->model) {
            def->ApplyMaterials(model, def->GetMaterialDef());
            if (shadow) def->ApplyMaterials(shadow, def->GetShadowMaterialDef());
            def->BuildAnimatedDummyList(model, ec->animatedDummies);
            jet::scene::Node &root = model->GetRootNode();
            root.SetPosition(position);
            root.SetRotation(def->RotateModel() ? rotation : math::quat<float>(0, 0, 0, 1));
            const math::vec3<float> &s = def->GetScale();
            root.SetScale(math::vec3<float>(s.x * scale.x, s.y * scale.y, s.z * scale.z));
            model->CC().SetLooping(def->IsLooped());
            model->CC().SetFinished(false);
            if (jet::anim::Animation *anim = jet::anim::AnimationLoader::GetInstance()->Load(def->GetAnimName())) model->SetAnimation(anim, 0);
        }
    }

    ec->hasParticles = false;
    if (!def->GetParticleSystem().empty()) {
        boost::shared_ptr<ps::ParticleSystem> particles = ps::ParticleMgr::GetInstance()->Load(def->GetParticleSystem());
        if (particles) {
            ec->hasParticles = true;
            ec->particles = particles;
            particles->Reset();
            // the level's particles are updated and drawn by the particle manager, those of a menu scene by the layer's own Update and Render
            const bool byManager = layerId == 0;
            particles->SetAutoRendering(byManager);
            particles->SetAutoUpdate(byManager);
            particles->SetEnabled(true);
            particles->SetPosition(position);
            particles->SetRotation(rotation);
            ec->position = position;
            for (unsigned i = 0; i < particles->GetEmitterCount(); i++) particles->GetEmitter(i).GetParticleDef().GetMaterial().SetDrawLayer(10);
        }
    }

    if (LightDef *light = def->GetLightDef()) {
        ec->light = light;
        light->StartEffectControl(ec);
        light->SetPosition(position);
        ec->lightTime = def->GetLightTime();
    }
    // the trail of the effect (TrailMgr::GetTrail, Trail::SetTrailDef) comes with the TrailMgr
    ec->active = true;
    ec->visible = true;
    InitBounceScreenAnim(ec);
    m_lastAdded = ec;
    return true;
}

// @0x59bff4
void EffectMgr::Release(EffectControl *ec) {
    if (!ec) return;
    if (ec->model) {
        ModelPool::GetInstance()->Release(ec->model);
        ec->model = 0;
    }
    ec->animatedDummies.clear();
    if (ec->hasParticles) {
        ec->particles.reset();
        ec->hasParticles = false;
    }
    ec->soundHandle = -1;  // the effect's sound stops with the SoundMgr
    if (ec->light) {
        ec->light->StopEffectControl();
        ec->light = 0;
    }
    ec->node = 0;
    SetBounceScreenState(ec, EffectControl::BounceScreenState::None);
    ec->active = false;
    ec->tag = 0;
    if (ec->listener) {
        ec->listener->ECL_OnRelease(ec);
        ec->listener = 0;
    }
}

// @0x59cd48
void EffectMgr::Stop(EffectControl *ec) {
    if (!ec) return;
    if (ec->model) {
        ModelPool::GetInstance()->Release(ec->model);
        ec->model = 0;
    }
    ec->animatedDummies.clear();
    if (ec->hasParticles && ec->particles->IsEnabled()) ec->particles->Stop(false);
    ec->soundHandle = -1;
    if (ec->light) {
        ec->light->StopEffectControl();
        ec->light = 0;
    }
    ec->node = 0;
    SetBounceScreenState(ec, EffectControl::BounceScreenState::None);
    ec->autoRemove = true;
    ec->tag = 0;
    if (ec->listener) {
        ec->listener->ECL_OnStop(ec);
        ec->listener = 0;
    }
}

// @0x59c240
void EffectMgr::Update(int layerId, int dt) {
    if (layerId == 0) {
        // the level's effects wait while the game is paused, in a cinematic, in the revive menu and during the countdown before a run
        Game *game = Game::GetInstance();
        if (game->IsPaused() || game->IsCinematic() || game->IsInDeathRevive()) dt = 0;
        else if (GS_Gameplay *gameplay = GS_Gameplay::GetInstance())
            if (gameplay->GetInterface()->IsCountdownEnabled()) dt = 0;
    }
    jet::scene::Camera *camera = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World)->GetCamera();
    for (int i = 0; i < k_maxEffects; i++) {
        EffectControl *ec = &m_controls[i];
        if (ec->layerId != layerId || !ec->active || !ec->visible) continue;
        const EffectDef *def = ec->def;
        // the sound of the effect (SoundMgr::Play3D within hearing range, Stop out of it) comes with the SoundMgr
        if (ec->node) {
            SetPosition(ec, ec->node->GetAbsolutePosition());
            SetRotation(ec, ec->node->GetAbsoluteRotation());
        }
        if (ec->hasParticles) ec->position = ec->particles->GetPosition();
        if (ec->lifeTime > 0) ec->lifeTime = std::max(0, ec->lifeTime - dt);
        if (ec->hasParticles && ec->particles->IsEnabled()) ec->particles->Update((unsigned)dt);
        if (jet::scene::Model *model = ec->model) {
            if (model->GetAnimation() || model->GetTextureAnimation()) model->Update(dt);
            def->UpdateShaderUniformByAnimatedDummy(ec->animatedDummies, model);
            jet::scene::Node &root = model->GetRootNode();
            ec->position = root.GetPosition();
            // an impostor turns to the camera, about the vertical axis
            if (def->IsImpostor() && def->RotateModel()) {
                math::vec3<float> toCamera = camera->GetPosition() - ec->position;
                toCamera.z = 0;
                root.SetRotation(math::quat<float>(math::lookAt(toCamera, math::vec3<float>(0, 0, 1))));
            }
            // the animation of the model is over (a bouncing one is released with the effect)
            if (model->CC().IsFinished() && !model->CC().IsLooping() && !def->GetUseBounceScreenAnim()) {
                ModelPool::GetInstance()->Release(model);
                ec->model = 0;
            }
        }
        if (ec->lightTime > 0) {
            ec->lightTime = std::max(0, ec->lightTime - dt);
            if (ec->lightTime == 0 && ec->light) ec->light->SetEnabled(false);
        }
        if (def->GetUseBounceScreenAnim()) UpdateBounceScreenAnim(ec, dt);
        if (ec->lifeTime == 0) {
            // the time of the effect is over: the animation of the model ends and the particles stop emitting
            if (ec->model) {
                ec->model->CC().SetLooping(false);
                ec->model->CC().SetFinished(true);
            }
            if (ec->hasParticles) ec->particles->Stop(false);
            ec->lifeTime = -1;
        }
        if (!ec->autoRemove) continue;
        // released when everything it plays has ended
        if (ec->model && (!ec->model->CC().IsFinished() || ec->model->CC().IsLooping())) continue;
        if (ec->hasParticles && !ec->particles->IsDone()) continue;
        if (ec->light && ec->lifeTime > 0) continue;
        if (def->GetUseBounceScreenAnim() && ec->bounceState != EffectControl::BounceScreenState::Done) continue;
        Release(ec);
    }
}

// @0x59c13c
void EffectMgr::Render(int layerId) {
    for (size_t i = 0; i < m_bounceScreenEffects.size(); i++)
        if (m_bounceScreenEffects[i] && m_bounceScreenEffects[i]->layerId == layerId) UpdateBounceScreenPosition(m_bounceScreenEffects[i]);
    if (layerId == 0) return;
    for (int i = 0; i < k_maxEffects; i++)
        if (m_controls[i].layerId == layerId && m_controls[i].hasParticles) m_controls[i].particles->Render();
}

// @0x59c1e8
bool EffectMgr::Particles_HasParticlesFor(int layerId) const {
    if (layerId == 0) return false;
    for (int i = 0; i < k_maxEffects; i++)
        if (m_controls[i].layerId == layerId && m_controls[i].hasParticles) return true;
    return false;
}

// @0x59c86c
void EffectMgr::SetPosition(EffectControl *ec, const math::vec3<float> &position) {
    if (!ec) return;
    if (ec->model) ec->model->GetRootNode().SetPosition(position);
    // the trail follows the points TrailA_Dummy and TrailB_Dummy of the model (TrailMgr)
    if (ec->hasParticles) ec->particles->SetPosition(position);
    if (ec->light) ec->light->SetPosition(position);
}

// @0x59ca18
void EffectMgr::SetRotation(EffectControl *ec, const math::quat<float> &rotation) {
    if (!ec) return;
    if (ec->model && ec->def->RotateModel()) ec->model->GetRootNode().SetRotation(rotation);
    if (ec->hasParticles) ec->particles->SetRotation(rotation);
}

// @0x59ca70
void EffectMgr::SetTransform(EffectControl *ec, const math::vec3<float> &position, const math::quat<float> &rotation) {
    if (!ec) return;
    if (ec->model) {
        ec->model->GetRootNode().SetPosition(position);
        if (ec->def->RotateModel()) ec->model->GetRootNode().SetRotation(rotation);
    }
    if (ec->hasParticles) {
        ec->particles->SetPosition(position);
        ec->particles->SetRotation(rotation);
    }
    if (ec->light) ec->light->SetPosition(position);
}

// @0x59cc1c
void EffectMgr::SetLinearVelocity(EffectControl *ec, const math::vec3<float> &velocity) {
    if (ec && ec->hasParticles) ec->particles->SetLinearVelocity(velocity);
}

// @0x59cc3c: hiding takes the model back to the pool and stops showing the particles
void EffectMgr::SetVisible(EffectControl *ec, bool visible) {
    if (!ec || ec->visible == visible) return;
    ec->visible = visible;
    if (ec->model) {
        if (!visible) {
            ModelPool::GetInstance()->Release(ec->model);
            ec->model = 0;
        } else {
            jet::scene::Model *shadow = 0;
            ec->model = ModelPool::GetInstance()->GetModel(ec->layerId, ec->def->GetModelName(), ec->def->GetShadowModelName(), &shadow);
            ec->def->ApplyMaterials(ec->model, ec->def->GetMaterialDef());
            if (shadow) ec->def->ApplyMaterials(shadow, ec->def->GetShadowMaterialDef());
        }
    }
    if (ec->hasParticles) ec->particles->SetEnabled(visible);
}

// @0x59ce84
void EffectMgr::Translate(const math::vec3<float> &delta) {
    for (int i = 0; i < k_maxEffects; i++)
        if (m_controls[i].active && m_controls[i].visible) SetPosition(&m_controls[i], m_controls[i].position + delta);
}

// @0x59ceec
void EffectMgr::OnModelChanged(jet::scene::Model *oldModel, jet::scene::Model *newModel) {
    for (int i = 0; i < k_maxEffects; i++) {
        EffectControl *ec = &m_controls[i];
        if (!ec->active || !ec->visible || !ec->node || ec->node->GetModel() != oldModel) continue;
        ec->node = newModel ? newModel->FindNodeByName(ec->node->GetName()) : 0;
    }
}

// @0x59cf6c
void EffectMgr::CleanUpAutoRemoveEffects(bool release) {
    for (int i = 0; i < k_maxEffects; i++) {
        if (!m_controls[i].active || !m_controls[i].autoRemove) continue;
        if (release) Release(&m_controls[i]);
        else Stop(&m_controls[i]);
    }
}

// @0x59cfcc
void EffectMgr::ReleaseAutoRemoveEffectsAttachedToModel(jet::scene::Model *model) {
    for (int i = 0; i < k_maxEffects; i++) {
        EffectControl *ec = &m_controls[i];
        if (ec->active && ec->autoRemove && ec->node && ec->node->GetModel() == model) Release(ec);
    }
}

// @0x59d060
void EffectMgr::OnEffectDefDestroyed(EffectDef *def) {
    for (int i = 0; i < k_maxEffects; i++)
        if (m_controls[i].def == def) Release(&m_controls[i]);
}

// @0x59bac0: an effect that bounces on the screen flies in from a side first, to a place chosen at random on that side
void EffectMgr::InitBounceScreenAnim(EffectControl *ec) {
    SetBounceScreenState(ec, EffectControl::BounceScreenState::None);
    const EffectDef *def = ec->def;
    if (!def->GetUseBounceScreenAnim()) return;
    SetBounceScreenState(ec, EffectControl::BounceScreenState::Flying);
    int side = def->GetBounceSide();
    if (side == EffectDef::BounceSide::Random) side = (int)jet::core::Rand() % 2;
    Game *game = Game::GetInstance();
    if (side == EffectDef::BounceSide::Left) ec->bounceX = game->GetDesignWidth() * -0.4375f;
    else if (side == EffectDef::BounceSide::Right) ec->bounceX = game->GetDesignWidth() * 0.4375f;
    const float a = game->GetDesignWidthScale() * -0.4375f, b = game->GetDesignWidthScale() * 0.4375f;
    const float low = std::min(a, b), high = std::max(a, b);
    ec->bounceY = low + (high - low) * (float)(jet::core::Rand() & 0xffff) * 1.52587890625e-05f;
}

// @0x59bbc8: the effect goes from state to state: Flying (the model flies in), Bouncing (it plays a bounce animation on the screen), Done
void EffectMgr::SetBounceScreenState(EffectControl *ec, int state) {
    ec->bounceState = state;
    ec->bounceTime = 0;
    if (state == EffectControl::BounceScreenState::Flying || state == EffectControl::BounceScreenState::Bouncing) {
        const bool flying = state == EffectControl::BounceScreenState::Flying;
        const std::vector<jet::String> &anims = flying ? ec->def->GetFlyingScreenAnim() : ec->def->GetBounceScreenAnim();
        if (ec->model && !anims.empty()) {
            ec->model->SetAnimation(jet::anim::AnimationLoader::GetInstance()->Load(anims[jet::core::Rand() % anims.size()]), 100);
            ec->model->CC().SetLooping(flying);
            ec->model->CC().SetFinished(false);
        }
        if (flying) {
            ec->bounceCamera = CameraMgr::GetInstance()->GetCurrentController();
            m_bounceScreenEffects.push_back(ec);
        }
    } else {
        std::vector<EffectControl*>::iterator it = std::find(m_bounceScreenEffects.begin(), m_bounceScreenEffects.end(), ec);
        if (it != m_bounceScreenEffects.end()) m_bounceScreenEffects.erase(it);
    }
}

// @0x59bd6c
void EffectMgr::UpdateBounceScreenAnim(EffectControl *ec, int dt) {
    if (ec->bounceState == EffectControl::BounceScreenState::None) return;
    if (ec->bounceState == EffectControl::BounceScreenState::Flying) {
        ec->bounceTime += (float)dt;
        if (ec->bounceTime >= k_bounceMoveToScreenTime) SetBounceScreenState(ec, EffectControl::BounceScreenState::Bouncing);
    } else if (ec->bounceState == EffectControl::BounceScreenState::Bouncing) {
        if (ec->model->CC().IsFinished()) SetBounceScreenState(ec, EffectControl::BounceScreenState::Done);
    }
    // the camera changed under it: the effect goes
    if (ec->bounceState != EffectControl::BounceScreenState::Flying && ec->bounceState != EffectControl::BounceScreenState::Bouncing) return;
    if (ec->bounceCamera == CameraMgr::GetInstance()->GetCurrentController()) return;
    SetBounceScreenState(ec, EffectControl::BounceScreenState::Done);
    Release(ec);
}

// @0x59be4c: the effect sits in front of the camera, where its place on the screen is; it flies there from where it is
void EffectMgr::UpdateBounceScreenPosition(EffectControl *ec) {
    CameraMgr *cameraMgr = CameraMgr::GetInstance();
    if (ec->bounceCamera != cameraMgr->GetCurrentController()) return;
    Game *game = Game::GetInstance();
    jet::scene::Camera *camera = cameraMgr->GetSceneCamera();
    const float scale = Sprite::GetDefaultPositionScale();
    const math::vec2<float> screen((ec->bounceX + game->GetDesignWidth() * 0.5f) * scale, (ec->bounceY + game->GetDesignWidthScale() * 0.5f) * scale);
    math::line3<float> ray = camera->GetRayFromScreenCoordinates(screen);
    const math::vec3<float> direction = ray.getVectorNormalized();
    const float distance = camera->GetNearDistance() + 0.5f;
    const math::vec3<float> position = ray.start + direction * distance;
    const math::quat<float> rotation = camera->GetRotation();
    if (ec->bounceState == EffectControl::BounceScreenState::Bouncing) {
        SetPosition(ec, position);
        SetRotation(ec, rotation);
    } else if (ec->bounceState == EffectControl::BounceScreenState::Flying) {
        const float t = std::min(ec->bounceTime * 0.004999999888241291f, 1.0f);
        SetPosition(ec, math::lerp(ec->model->GetRootNode().GetPosition(), position, t));
    }
}
