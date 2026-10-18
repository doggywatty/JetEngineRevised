// source/game/gamemanagers/FlareMgr.cpp
#include "FlareMgr.h"
#include <algorithm>
#include <cfloat>
#include "../CGameConfig.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../entities/LensFlareDef.h"
#include "../entities/LensFlareEntity.h"
#include "PainterMgr.h"
#include "physics/physicsworld.h"
#include "scene/camera.h"
#include "sprites/sprite.h"
#include "video/material.h"
#include "video/painter.h"

// @0x54a69c
FlareMgr::FlareMgr() : m_enabled(false), m_frame(0), m_period(10), m_ms(0), m_visible(0) {}

// @0x54a7cc
void FlareMgr::AddFlare(LensFlareEntity *e, const math::vec3<float> &pos, const math::quat<float> &rot) {
    Flare *f = new Flare;
    f->entity = e;
    e->SetRenderable(false);
    f->position = pos;
    f->rotation = rot;
    f->occluded = false;
    m_flares.push_back(f);
}

// @0x54a900
void FlareMgr::CleanFlares() {
    for (Flare *f : m_flares) delete f;
    m_flares.clear();
}

// @0x54a934: the flares in sight first (and enabled), the nearest to the camera first
static bool FlaresPriorityPredicate(const FlareMgr::Flare *a, const FlareMgr::Flare *b) {
    const math::vec3<float> cam = GameLevel::GetInstance()->GetCamera()->GetPosition();
    LensFlareEntity *ea = a->entity, *eb = b->entity;
    if (!ea->IsEnabled() || !ea->IsVisible()) return false;
    if (!eb->IsEnabled() || !eb->IsVisible()) return true;
    const math::vec3<float> pa = a->position + a->rotation * ea->GetPosition();
    const math::vec3<float> pb = b->position + b->rotation * eb->GetPosition();
    return (pa - cam).getLengthSq() < (pb - cam).getLengthSq();
}

// @0x54aa90
void FlareMgr::SortFlaresByDistance() { std::sort(m_flares.begin(), m_flares.end(), FlaresPriorityPredicate); }

// @0x54ab38: every m_period frames each flare's light is looked for from the camera: in front of it, close enough
// (the entity's MinDist) and with nothing between (a ray cast from the camera's near plane to the light); the ones
// seen are renderable, the others start their fade in again
void FlareMgr::Update(unsigned ms) {
    if (!CGameConfig::Current().useUIDespicableScoreParticles) return;  // Game+0x400, the last of the platform toggles
    m_ms = ms;
    if (m_period != ++m_frame) return;
    m_frame = 0;
    m_visible = 0;
    SortFlaresByDistance();
    GameLevel *level = GameLevel::GetInstance();
    jet::scene::Camera *camera = level->GetCamera();
    const math::vec3<float> camPos = camera->GetPosition();
    const math::vec3<float> front = camera->GetFrontVector();
    PhysicsWorld *world = level->GetPhysicsWorld();
    static RaycastResult hit;
    hit.distance = FLT_MAX;
    for (Flare *f : m_flares) {
        LensFlareEntity *e = f->entity;
        const math::vec3<float> light = f->position + f->rotation * e->GetPosition();
        const math::vec3<float> toLight = light - camPos;
        const float facing = math::dot(toLight, camera->GetFrontVector());
        e->SetRenderable(false);
        if (facing > 0 && toLight.getLengthSq() < e->GetMinDistSq()) {
            const math::vec3<float> from = camPos + front * camera->GetNearDistance();
            // @0x66e720 BulletPhysicsWorld::RayCast: the groups 1 and 2; without a physics world nothing is in the way
            f->occluded = world ? world->RayCast(from, light, 1, 2, hit) : false;
            if (!f->occluded) {
                e->SetRenderable(true);
                m_visible++;
            }
        }
        if (!e->IsVisible()) {
            f->elapsed = 0;
            f->from = 0;
            f->current = 0;
            f->to = 1.0f;
            f->fading = !math::equals(0.0f, 1.0f, 1.1920929e-07f);
        }
    }
}

// @0x54ae18: the flares in sight over the scene: a sprite per element of its definition, along the line from the
// light's place on the screen to the screen's middle, each at the light's brightness; the fade in of a flare eases
// its factor to 1 (quartic) over its fade time, and a light seen too edge on (or off the screen) does not draw
void FlareMgr::Render() {
    if (!CGameConfig::Current().useUIDespicableScoreParticles || !m_enabled) return;
    jet::scene::Camera *camera = GameLevel::GetInstance()->GetCamera();
    const math::vec3<float> camPos = camera->GetPosition();
    const math::vec3<float> front = camera->GetFrontVector();
    jet::video::Painter &painter = PainterMgr::GetInstance()->GetGenericPainter();
    const jet::video::Material saved(painter.GetMaterial());
    const int dt = (int)Game::GetInstance()->GetRenderTime();
    for (Flare *f : m_flares) {
        LensFlareEntity *e = f->entity;
        const math::vec3<float> light = f->position + f->rotation * e->GetPosition();
        if (!e->IsVisible() || !e->IsEnabled() || !m_enabled) continue;
        LensFlareDef *def = e->GetDef();
        if (!def) continue;
        const math::vec3<float> end = camPos + front * camera->GetNearDistance();
        if (f->fading) {
            f->elapsed += dt;
            if (f->elapsed >= f->fadeTime) {
                f->current = f->to;
                f->fading = false;
            } else if (f->elapsed < 0) {
                f->current = f->from;
            } else {
                const float k = 1.0f - (float)f->elapsed / (float)f->fadeTime;
                const float k2 = k * k, k4 = k2 * k2;
                f->current = f->from + (1.0f - k4 * k4) * (f->to - f->from);
            }
        }
        if (f->fadeTime > 0) {
            if (!math::equals(f->to, 1.0f, 1.1920929e-07f)) {
                f->from = f->current;
                f->to = 1.0f;
                f->elapsed = 0;
                f->fading = !math::equals(f->current, 1.0f, 1.1920929e-07f);
            }
        } else {
            f->current = 1.0f;
            f->to = 1.0f;
            f->fading = false;
        }
        math::vec3<float> dir = light - camPos;
        dir.normalize();
        if (math::dot(dir, front) * f->current < 1.0f / 256.0f) {
            m_visible--;
            continue;
        }
        math::vec3<float> lightOnScreen(0, 0, 0), middle(0, 0, 0);
        if (!camera->Project(lightOnScreen, light)) {
            m_visible--;
            continue;
        }
        const unsigned v = (unsigned)(def->GetIntensity() * 255.0f);
        const unsigned packed = (v & 0xff) | ((v & 0xff) << 8) | ((v & 0xff) << 16) | (v << 24);
        camera->Project(middle, end);
        const std::vector<LensFlareDef::Element> &elements = def->GetElements();
        for (size_t i = 0; i < elements.size(); i++) {
            const LensFlareDef::Element &el = elements[i];
            el.sprite->SetCurrentColor(jet::video::Color(packed));
            Sprite::SetBlending(Sprite::SpriteBlending((Sprite::SpriteBlendingDef::type)el.blending));
            const math::vec3<float> pos = lightOnScreen + (middle - lightOnScreen) * (el.distance + el.distance);
            math::mat3<float> m;
            m.m[0] = m.m[4] = el.scale;
            m.m[1] = m.m[2] = m.m[3] = m.m[5] = 0;
            m.m[6] = pos.x;
            m.m[7] = pos.y;
            m.m[8] = 1.0f;
            el.sprite->_PaintPrepareRender(painter);
            el.sprite->_PaintFrame(painter, el.frame, m);
            el.sprite->_PaintRenderDone();
            if (i == elements.size() - 1) Sprite::SetBlending(Sprite::SpriteBlending((Sprite::SpriteBlendingDef::type)4));
        }
    }
    painter.SetMaterial(saved);
    painter.Flush();
}
