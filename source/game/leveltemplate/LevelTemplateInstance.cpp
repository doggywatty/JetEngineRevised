// source/game/leveltemplate/LevelTemplateInstance.cpp
#include <algorithm>
#include <cmath>
#include "game/leveltemplate/LevelTemplateInstance.h"
#include "game/GameLevel.h"
#include "game/entities/GameEntityCreator.h"
#include "game/entities/TouchInteract3d.h"
#include "game/utils/ControlledRandom.h"

float LevelTemplateInstance::s_lastAllowedDistancePosition = 0;

static const float k_degToRad = 0.017453292f;

// @0x3e8818
LevelTemplateInstance::LevelTemplateInstance(LevelTemplateDef *def)
    : m_def(def), m_isEnabled(true), m_lastEntityParsed(0), m_startPos(0, 0, 0), m_startAngle(0), m_endPos(0, 0, 0), m_endAngle(0), m_bifurcationDecided(false),
      m_bifurcationType(0), m_volumeMin(-1, -1, -1), m_volumeMax(1, 1, 1), m_dir(0, 0, 0), m_distanceToRemove(0) {}

// @0x3e0234: the signed distance of a point to the plane across the instance's start (moved along its direction by scale)
float LevelTemplateInstance::PlaneDistance(const math::vec3<float> &p, float scale) const {
    math::quat<float> rot(math::quat<float>::axisZ, m_startAngle * k_degToRad);
    math::vec3<float> d = math::transform(rot.getAsMat3(), math::vec3<float>(1, 0, 0));
    math::vec3<float> plane = m_startPos + d * scale;
    return d.x * p.x + d.y * p.y + d.z * p.z - d.x * plane.x - d.z * plane.z - d.y * plane.y;
}

// @0x3e01dc
bool LevelTemplateInstance::Contains(const math::vec3<float> &p) const {
    return m_volumeMin.x <= p.x && p.x <= m_volumeMax.x && m_volumeMin.y <= p.y && p.y <= m_volumeMax.y;
}

// @0x3e8948: the end anchor and the volume keep their place relative to the start
void LevelTemplateInstance::SetStartAnchor(const math::vec3<float> &pos, float angle) {
    math::vec3<float> endDelta = m_endPos - m_startPos;
    float angleDelta = m_endAngle - m_startAngle;
    math::vec3<float> move = pos - m_startPos;
    m_startPos = pos;
    m_startAngle = angle;
    m_endPos = pos + endDelta;
    m_endAngle = angleDelta + angle;
    m_volumeMin += move;
    m_volumeMax += move;
}

// the volume of the template's rectangle (one unit tall) turned by rot at the start
void LevelTemplateInstance::UpdateVolume(const math::vec3<float> &start, const math::quat<float> &rot) {
    math::mat3<float> m = rot.getAsMat3();
    float halfX = m_def->GetSizeX() * 0.5f, halfY = m_def->GetSizeY() * 0.5f;
    math::vec3<float> c = start + math::transform(m, math::vec3<float>(halfX, 0, 0));
    math::vec3<float> k = math::transform(m, math::vec3<float>(halfX, halfY, 1));
    m_volumeMin = c - k;
    m_volumeMax = c + k;
    if (m_volumeMax.x <= m_volumeMin.x) std::swap(m_volumeMin.x, m_volumeMax.x);
    if (m_volumeMax.y <= m_volumeMin.y) std::swap(m_volumeMin.y, m_volumeMax.y);
}

// @0x3e8a98
unsigned LevelTemplateInstance::SpawnEntities(int count, GameplayTemplateInstance *gti) {
    unsigned index = m_lastEntityParsed;
    unsigned stop = m_def->GetEntityCount();
    if (count >= 0 && index + (unsigned)count < stop) stop = index + (unsigned)count;
    math::vec3<float> start = m_startPos;
    float angle = m_startAngle;
    math::quat<float> rot(math::quat<float>::axisZ, angle * k_degToRad);
    math::mat3<float> m = rot.getAsMat3();
    math::vec3<float> originOffset = math::transform(m, math::vec3<float>(m_def->GetSizeX() * 0.5f - m_def->GetOffsetX(), 0, 0));
    UpdateVolume(start, rot);
    if (index == 0) BuildAllowedDecoList(0);
    for (; index < stop; index++) {
        GameEntity *tmpl = m_def->GetEntity(index);
        if (!tmpl->CanSpawnNewEntity(this)) continue;
        GameEntityCreator creator;
        GameEntity *copy = dynamic_cast<GameEntity*>(creator.CreateEntity(tmpl->GetTemplate()));
        copy->ConstructFrom(static_cast<const GameEntity*>(tmpl));
        math::vec3<float> pos0 = copy->GetPosition();
        math::quat<float> rot0 = copy->GetRotation();
        if (!copy->IsLinked()) {
            copy->SetRotation(rot * copy->GetRotation());
            copy->SetPosition(start + math::transform(m, copy->GetPosition()) + originOffset);
        }
        copy->OnPositionedInWorld();
        copy->SetPersistenceType(GameEntity::PersistenceTypeDef::Volatile);
        copy->Update();
        GameEntity *made = ProcessCreatedEntity(tmpl, copy, gti);
        if (copy->GetType() == 0xc && copy->GetLinkedEntitiesCount() != 0) stop = std::min<unsigned>(stop + 1, m_def->GetEntityCount());
        if (made) {
            GameLevel::GetInstance()->RemoveEntities();
            Instance in = { made, pos0, rot0, false };
            m_createdEntities.push_back(in);
        }
    }
    // the end: a turn at a bifurcation, else straight on
    math::vec3<float> end = m_def ? start : start;
    math::vec3<float> off(m_def->GetSizeX() * 0.5f - m_def->GetOffsetX(), 0, 0);
    switch (m_bifurcationType) {
    case 1: angle -= 90.0f; off = math::vec3<float>(m_def->GetSizeX() * 0.5f, m_def->GetSizeY() * 0.5f, 0); break;
    case 2: angle += 90.0f; off = math::vec3<float>(m_def->GetSizeX() * 0.5f, -(m_def->GetSizeY() * 0.5f), 0); break;
    case 0: off = math::vec3<float>(m_def->GetSizeX(), 0, 0); break;
    default: break;
    }
    end = start + math::transform(m, off);
    m_endAngle = angle;
    m_lastEntityParsed = stop;
    m_endPos = end;
    return stop;
}

// @0x3e9024
void LevelTemplateInstance::Translate(const math::vec3<float> &d) {
    SetStartAnchor(m_startPos + d, m_startAngle);
    for (std::list<Instance>::iterator i = m_createdEntities.begin(); i != m_createdEntities.end(); ++i) i->entity->Translate(d);
}

// @0x3e9084
void LevelTemplateInstance::Reposition(const math::vec3<float> &pos, float angle) {
    math::quat<float> rot(math::quat<float>::axisZ, angle * k_degToRad);
    math::mat3<float> m = rot.getAsMat3();
    math::vec3<float> originOffset = math::transform(m, math::vec3<float>(m_def->GetSizeX() * 0.5f - m_def->GetOffsetX(), 0, 0));
    UpdateVolume(pos, rot);
    for (std::list<Instance>::iterator i = m_createdEntities.begin(); i != m_createdEntities.end(); ++i) {
        i->entity->SetRotation(rot * i->rotation);
        i->entity->SetPosition(pos + math::transform(m, i->position) + originOffset);
    }
}

// @0x3e92bc
void LevelTemplateInstance::SetPendingToDelete(unsigned distance) {
    for (std::list<Instance>::iterator i = m_createdEntities.begin(); i != m_createdEntities.end(); ++i) i->entity->SetPendingToDelete(distance);
}

// @0x3e92ec
void LevelTemplateInstance::EnableTouchInteracts(bool b) {
    for (std::list<Instance>::iterator i = m_createdEntities.begin(); i != m_createdEntities.end(); ++i) {
        GameEntity *e = i->entity;
        if (e->GetType() == GameEntity::EntityTypeDef::TouchInteract3d) {
            TouchInteract3d *t = static_cast<TouchInteract3d*>(e);
            t->BPO_Enable(b);
            t->Reset(b);
        }
    }
}

// @0x3e9338
void LevelTemplateInstance::DeleteEntities() {
    for (std::list<Instance>::iterator i = m_createdEntities.begin(); i != m_createdEntities.end(); ++i)
        if (i->entity) i->entity->Destroy();
    m_createdEntities.clear();
}

// @0x3e9384
bool LevelTemplateInstance::IsEntityAllowed(const GameEntity::GenerationGroupEntity *e) const {
    if (!m_def || !e || e->group == 0) return true;
    LevelTemplateDef::EntityGenerationGroup &group = m_def->GetEntityGenerationGroup(e->group);
    LevelTemplateDef::GenerationMap::const_iterator it = m_entityGenerationGroups.find(e->group);
    if (it == m_entityGenerationGroups.end()) return false;
    bool allowed = false;
    for (size_t k = 0; k < it->second.allowed.size(); k++)
        if (group.entities[it->second.allowed[k]] == e) allowed = true;
    return allowed;
}

// @0x3e9448: for each group (or the one asked for) the decos to place: the forced ones, then random ones far enough from
// each other and from the last group's, up to the group's maximum
void LevelTemplateInstance::BuildAllowedDecoList(unsigned groupFilter) {
    m_def->InitEntityGenerationMap(m_entityGenerationGroups);
    ControlledRandom *rnd = GameLevel::s_randomizer[2];
    for (LevelTemplateDef::GenerationMap::iterator it = m_entityGenerationGroups.begin(); it != m_entityGenerationGroups.end(); ++it) {
        if (groupFilter != 0 && groupFilter != it->first) continue;
        LevelTemplateDef::EntityGenerationGroup &group = m_def->GetEntityGenerationGroup(it->first);
        std::vector<unsigned> &allowed = it->second.allowed;
        allowed.clear();
        std::vector<unsigned> remaining;
        for (unsigned i = 0; i < group.entities.size(); i++) remaining.push_back(i);
        unsigned forcedCount = 0;
        float maxPos = 0;
        for (size_t i = 0; i < remaining.size();) {
            const GameEntity::GenerationGroupEntity *e = group.entities[remaining[i]];
            if (!e->forced) { i++; continue; }
            forcedCount++;
            maxPos = std::max(maxPos, e->position);
            allowed.push_back(remaining[i]);
            remaining.erase(remaining.begin() + i);
        }
        unsigned total = std::min<unsigned>((unsigned)group.entities.size(), group.maxDecos);
        unsigned want = total - forcedCount;
        if (total <= want) want = total;
        for (unsigned picked = 0; picked != want; picked++) {
            unsigned n = (unsigned)remaining.size();
            unsigned idx = (unsigned)rnd->Rand() % n;
            float pos = 0;
            unsigned tries = 0;
            bool ok;
            do {
                idx = (idx + 1) % n;
                const GameEntity::GenerationGroupEntity *e = group.entities[remaining[idx]];
                pos = e->position;
                ok = !(std::fabs(s_lastAllowedDistancePosition - pos) < group.minDistance);
                for (size_t a = 0; a < allowed.size(); a++) {
                    const GameEntity::GenerationGroupEntity *o = group.entities[allowed[a]];
                    if (o != e && std::fabs(o->position - pos) < group.minDistance) ok = false;
                }
            } while (!ok && ++tries < n);
            allowed.push_back(remaining[idx]);
            maxPos = std::max(maxPos, pos);
            remaining.erase(remaining.begin() + idx);
        }
        if (!allowed.empty()) s_lastAllowedDistancePosition = maxPos;
    }
}
