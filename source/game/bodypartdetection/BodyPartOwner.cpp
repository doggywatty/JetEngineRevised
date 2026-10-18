// source/game/bodypartdetection/BodyPartOwner.cpp
#include "BodyPartOwner.h"
#include "scene/model.h"
#include "scene/node.h"
#include "entity.h"
#include "param.h"
#include "scene/camera.h"
#include <algorithm>

std::vector<BodyPartOwner*> BodyPartOwner::s_damageables;
jet::String BodyPartOwner::k_SimpleBodyPartDefinition_tmplName("SimpleBodyPartDefinition");
jet::String BodyPartOwner::k_TouchInteractBodyPartDefinition_tmplName("TouchInteractBodyPartDefinition");

BodyPartDefinition::BodyPartDefinition()
    : radius(0), height(0), angle(0), cylinderCover(false), position(0, 0, 0), rotation(0, 0, 0, 1),
      type(0), touchEvent(0), linked3dLayer(0), baseRadius(0), baseHeight(0) {}

// IsReadyToCheck @0x500b58: enabled, and when a touch state is set, only while bit 8 is clear
bool BodyPartSection::IsReadyToCheck() const {
    if (!(flags & Enabled)) return false;
    if (flags & 6) return !(flags & 8);
    return true;
}

// GetStartAndEnd @0x5adb94: the part's axis in world space, from its definition's offset on the node
// (start) along the definition's rotated +Z by its height (end)
void BodyPartSection::GetStartAndEnd(math::vec3<float> &outStart, math::vec3<float> &outEnd) const {
    math::quat<float> q = node->GetAbsoluteRotation();
    math::vec3<float> p = node->GetAbsolutePosition() + q * def->position;
    outStart = p;
    math::quat<float> q2 = q * def->rotation;
    outEnd = p + q2 * math::vec3<float>(0, 0, def->height);
}

// GetStart @0x5adc8c: where the part's definition sits on the node, in world space
void BodyPartSection::GetStart(math::vec3<float> &outStart) const {
    math::quat<float> q = node->GetAbsoluteRotation();
    outStart = node->GetAbsolutePosition() + q * def->position;
}

BodyPartOwner::BodyPartOwner() : m_classType(ClassType_Default), m_hitSphereRadius(0), m_hitSpherePos(0, 0, 0), m_pModel(0), m_pOwner(0), m_sceneMgrId(0) {}

BodyPartOwner::~BodyPartOwner() {
    BPO_Enable(false);
    BPO_Release();
}

// BPO_Init @0x525010
void BodyPartOwner::BPO_Init(jet::scene::Model *m) {
    m_pOwner = this;
    m_pModel = m;
    m_hitSpherePos = m->GetBoundingSphereCenter();
    m_hitSphereRadius = m->GetBoundingSphereRadius();
}

// BPO_Enable @0x52503c: membership in the global damageable list
void BodyPartOwner::BPO_Enable(bool on) {
    std::vector<BodyPartOwner*>::iterator it = std::find(s_damageables.begin(), s_damageables.end(), this);
    if (on) {
        if (it == s_damageables.end()) s_damageables.push_back(this);
    } else if (it != s_damageables.end()) {
        s_damageables.erase(it);
    }
}

void BodyPartOwner::BPO_Release() {
    while (!m_bodyParts.empty()) BPO_RemoveBodyPart(m_bodyParts[0].def);
    m_pModel = 0;
}

void BodyPartOwner::BPO_SetModel(jet::scene::Model *m) {
    m_pModel = m;
}

// BPO_AddBodyPart @0x5251e0: bound to the model node named by NodeRef, enabled and visible
void BodyPartOwner::BPO_AddBodyPart(BodyPartDefinition *d) {
    const jet::scene::Node *n = m_pModel->FindNodeByName(d->nodeRef);
    if (!n) return;
    BodyPartSection s;
    s.node = const_cast<jet::scene::Node*>(n);
    s.def = d;
    s.flags = BodyPartSection::Enabled | BodyPartSection::Visible;
    m_bodyParts.push_back(s);
}

void BodyPartOwner::BPO_RemoveBodyPart(BodyPartDefinition *d) {
    for (size_t i = 0; i < m_bodyParts.size(); i++)
        if (m_bodyParts[i].def == d) {
            m_bodyParts.erase(m_bodyParts.begin() + i);
            return;
        }
}

void BodyPartOwner::BPO_LoadBodyPartsArray(std::vector<BodyPartDefinition> &defs, clara::Param *p) {
    if (!p) return;
    // the project can initialise an entity again (the libraries of a level are loaded after the menu's copies exist): the
    // definitions are read again where they are, as the sections point at them
    if (defs.size() == p->GetComponentCount()) {
        for (unsigned i = 0; i < p->GetComponentCount(); i++) BPO_LoadBodyPart(&defs[i], dynamic_cast<clara::Entity*>(p->GetAsEntity(i)));
        return;
    }
    m_lastHit.clear();
    m_bodyParts.clear();
    defs.clear();
    for (unsigned i = 0; i < p->GetComponentCount(); i++) {
        clara::Entity *e = dynamic_cast<clara::Entity*>(p->GetAsEntity(i));
        defs.push_back(BodyPartDefinition());
        BPO_LoadBodyPart(&defs.back(), e);
    }
}

void BodyPartOwner::BPO_AddBodyPartsArray(std::vector<BodyPartDefinition> &defs) {
    for (size_t i = 0; i < defs.size(); i++) BPO_AddBodyPart(&defs[i]);
}

// BPO_LoadBodyPart @0x5255f4
void BodyPartOwner::BPO_LoadBodyPart(BodyPartDefinition *d, clara::Entity *e) {
    static const math::quat<float> k_rot = math::quat<float>().setFromEulerXYZ(math::vec3<float>(0, -1.5707964f, 0));
    if (!e) return;
    d->name = e->GetName();
    e->GetParam(jet::String("NodeRef"), d->nodeRef);
    e->GetParam(jet::String("Radius"), d->radius);
    e->GetParam(jet::String("Height"), d->height);
    e->GetParam(jet::String("Angle"), d->angle);
    d->angle *= 0.0087266462f;
    e->GetParam(jet::String("cylinderCover"), d->cylinderCover);
    d->position = e->GetPosition();
    d->rotation = k_rot * e->GetRotation();
    int t = 0;
    e->GetParam(jet::String("Type"), t);
    d->type = t;
    e->GetParam(jet::String("touchEvent"), d->touchEvent);
    if (e->FindParamByName(jet::String("TouchedAnimation"))) {
        e->GetParam(jet::String("TouchedAnimation"), d->touchedAnimation);
        e->GetParam(jet::String("PressedAnimation"), d->pressedAnimation);
        e->GetParam(jet::String("TouchedSound"), d->touchedSound);
    }
    d->linked3dLayer = -1;
    if (e->FindParamByName(jet::String("linked3dLayer"))) {
        e->GetParam(jet::String("linked3dLayer"), d->linked3dLayer);
        d->linked3dLayer--;
    }
    d->baseRadius = d->radius;
    d->baseHeight = d->height;
}

void BodyPartOwner::BPO_SetBodyPartEnabled(BodyPartDefinition *d, bool on) {
    bool found = false;
    for (size_t i = 0; i < m_bodyParts.size() && !found; i++)
        if (m_bodyParts[i].def == d) {
            m_bodyParts[i].flags = on ? (m_bodyParts[i].flags | BodyPartSection::Enabled) : (m_bodyParts[i].flags & ~BodyPartSection::Enabled);
            found = true;
        }
    if (on && !found) BPO_AddBodyPart(d);
}

void BodyPartOwner::BPO_SetBodyPartVisible(BodyPartDefinition *d, bool visible, bool setCheck, bool check) {
    for (size_t i = 0; i < m_bodyParts.size(); i++) {
        if (m_bodyParts[i].def != d) continue;
        unsigned char &f = m_bodyParts[i].flags;
        f = visible ? (f | BodyPartSection::Visible) : (f & ~BodyPartSection::Visible);
        if (setCheck) f = check ? (f | BodyPartSection::Check4) : (f & ~BodyPartSection::Check4);
        return;
    }
}

void BodyPartOwner::BPO_AddToLastHit(BodyPartSection *s) {
    if (!BPO_IsLastHit(s)) m_lastHit.push_back(s);
}

bool BodyPartOwner::BPO_IsLastHit(BodyPartSection *s) const {
    return std::find(m_lastHit.begin(), m_lastHit.end(), s) != m_lastHit.end();
}

// BPO_Update @0x525afc: the hit sphere follows the model's bounding sphere
void BodyPartOwner::BPO_Update(int) {
    if (!m_pModel) return;
    m_hitSpherePos = m_pModel->GetBoundingSphereCenter();
    m_hitSphereRadius = m_pModel->GetBoundingSphereRadius();
}

void BodyPartOwner::BPO_Render() {
    for (size_t i = 0; i < m_bodyParts.size(); i++) BPO_RenderBodyPart(&m_bodyParts[i]);
}

// BPO_Scale @0x527208: every part's size scales by f from what it was loaded with
void BodyPartOwner::BPO_Scale(float f) {
    for (size_t i = 0; i < m_bodyParts.size(); i++) {
        m_bodyParts[i].def->radius = f * m_bodyParts[i].def->baseRadius;
        m_bodyParts[i].def->height = f * m_bodyParts[i].def->baseHeight;
    }
}

// BPO_GetBodyPartHit @0x58e5c8: the nearest of this owner's parts the segment touches. A part with a height is a
// cylinder along its axis, one without a sphere. Returns the part's index (-1 for none) and, when it
// found one, the point where the segment meets it in outHitPos. The segment is first made longer by the
// owner's hit sphere radius.
int BodyPartOwner::BPO_GetBodyPartHit(const jet::scene::Camera &cam, math::line3<float> line, math::vec3<float> &outHitPos, math::quat<float>&, bool addToLastHit) {
    if (m_bodyParts.empty()) return -1;
    math::vec3<float> rayDir = line.getVectorNormalized();
    line.end += rayDir * m_hitSphereRadius;
    const_cast<jet::scene::Camera&>(cam).GetFrontVector();  // the call only refreshes the camera
    math::vec3<float> best = outHitPos;
    float bestDistSq = 100000000.0f;
    int bestIndex = -1;
    bool found = false;
    math::vec3<float> center(0, 0, 0), hit(0, 0, 0);
    for (unsigned i = 0; i < m_bodyParts.size(); i++) {
        BodyPartSection *sec = &m_bodyParts[i];
        if (!sec->IsReadyToCheck()) continue;
        if (sec->def->height != 0.0f) {
            // a cylinder: where the segment crosses the plane through the axis (and square to the segment
            // with it) has to be within the radius of the axis
            math::vec3<float> a(0, 0, 0), b(0, 0, 0);
            sec->GetStartAndEnd(a, b);
            math::vec3<float> axis = b - a;
            math::vec3<float> n = math::cross(rayDir, axis.normalize());
            math::triangle3<float> plane(a, b, a + n);
            if (!plane.getIntersectionOfPlaneWithLine(line.start, rayDir, hit)) continue;
            if (!line.isPointBetweenStartAndEnd(hit)) continue;
            math::line3<float> axisLine(a, b);
            center = axisLine.getClosestPoint(hit);
            if (!axisLine.isPointBetweenStartAndEnd(center)) continue;
            if (!(math::distanceSq(hit, center) <= sec->def->radius * sec->def->radius)) continue;
            sec->hit[0] = hit.x; sec->hit[1] = hit.y; sec->hit[2] = hit.z;
            if (addToLastHit) BPO_AddToLastHit(sec);
            float d = math::distanceSq(line.start, hit);
            found = true;
            if (!(d < bestDistSq)) continue;
            best = hit - rayDir * sec->def->radius;
            bestDistSq = d;
            bestIndex = (int)i;
        } else {
            sec->GetStart(center);
            hit = line.getClosestPoint(center);
            if (math::distanceSq(center, hit) > sec->def->radius * sec->def->radius) continue;
            sec->hit[0] = hit.x; sec->hit[1] = hit.y; sec->hit[2] = hit.z;
            if (addToLastHit) BPO_AddToLastHit(sec);
            float d = math::distanceSq(line.start, hit);
            found = true;
            if (!(d < bestDistSq)) continue;
            best = line.getClosestPoint(center - line.getVectorNormalized() * sec->def->radius);
            bestDistSq = d;
            bestIndex = (int)i;
        }
    }
    if (bestIndex < 0) return -1;
    if (!m_lastHit.empty() || (found && !addToLastHit)) {
        outHitPos = best;
        return bestIndex;
    }
    return -1;
}

// BPO_GetProjectileHit @0x58df70: the nearest owner (of a class in ownerClassMask, not skipOwner nor owned by
// it) the segment rayStart-rayEnd goes through. The owners' hit spheres are tested first, then their body
// parts. outHitPos is where the nearest part was hit (the point on the sphere for an owner with no parts)
// and outHitRot looks along the segment.
BodyPartOwner *BodyPartOwner::BPO_GetProjectileHit(const jet::scene::Camera &cam, const math::vec3<float> &rayStart, const math::vec3<float> &rayEnd, math::vec3<float> &outHitPos, math::quat<float> &outHitRot, BodyPartOwner *skipOwner, bool clearHits, bool addToLastHit, unsigned char ownerClassMask) {
    math::line3<float> ray(rayStart, rayEnd);
    math::vec3<float> mid = (ray.start + ray.end) * 0.5f;
    float rayLen = math::distance(ray.start, ray.end);
    math::vec3<float> dir = ray.getVectorNormalized();
    math::vec3<float> facing(dir.x, dir.y, 0);
    math::vec3<float> up(-dir.x, -dir.y, -dir.z);
    outHitRot = math::quat<float>().setFromMat3(math::lookAt(facing, up));
    BodyPartOwner *nearest = 0;
    float bestDistSq = 999999.0f;
    unsigned count = (unsigned)s_damageables.size();
    for (unsigned i = 0; i < count; i++) {
        BodyPartOwner *o = s_damageables[i];
        if (skipOwner && (o == skipOwner || o->m_pOwner == skipOwner)) continue;
        if (!(ownerClassMask & o->m_classType)) continue;
        math::vec3<float> c = o->m_hitSpherePos;
        float r = o->m_hitSphereRadius;
        float reach = rayLen + r;
        if (math::distanceSq(mid, c) > reach * reach) continue;
        math::vec3<float> toC = c - ray.start;
        float toCSq = toC.getLengthSq();
        float t = math::dot(toC, ray.getVectorNormalized());
        float disc = t * t - toCSq + r * r;
        if (disc < 0) continue;
        float d = (float)((double)t - std::sqrt((double)disc));
        if (d < 0.001f) d = 0.001f;
        math::vec3<float> sphereHit = ray.start + dir * d;
        if (!ray.isPointBetweenStartAndEnd(sphereHit)) continue;
        float distSq = math::distanceSq(rayStart, sphereHit);
        if (!(bestDistSq > distSq)) continue;
        if (clearHits) o->m_lastHit.clear();
        int part = o->BPO_GetBodyPartHit(cam, ray, outHitPos, outHitRot, addToLastHit);
        if (o->m_bodyParts.empty()) {
            if (part == -1) outHitPos = sphereHit;
            bestDistSq = distSq;
            nearest = o;
        } else if (part >= 0) {
            bestDistSq = distSq;
            nearest = o;
        }
    }
    return nearest;
}
