// source/game/entities/Object.cpp
#include "Object.h"
#include "Deco3d.h"
#include "Collision3d.h"
#include "../GameLevel.h"
#include "param.h"
#include "project.h"
#include "core/util.h"
#include "game/dynamicobjects/EffectMgr.h"
#include "game/dynamicobjects/EffectDef.h"

const jet::String Object::k_tmplName("Object3d"), Object::k_objectDeco("objectDeco"), Object::k_objectCollision3d("objectCollision3d"),
    Object::k_effectOnContact("effectOnContact"), Object::k_effectOnContactSpawnAll("effectOnContactSpawnAll"), Object::k_movieLinkedDeco("movieLinkedDeco");

// @0x57efa4
Object::Object(const clara::Template *t)
    : GameEntity(t), m_decoTransferred(true), m_ctorFlags(0), m_effectLayerId(0), m_deco(0), m_decoReference(0), m_defaultDeco(0), m_decoIndex(0), m_effectOnContactSpawnAll(true) {
    m_type = 7;
}

// @0x57f16c
void Object::Init() { GameEntity::Init(); }

// @0x57f1b8
void Object::PostInit() {
    GameEntity::PostInit();
    if (IsLibraryEntity()) return;
    InitCollision3D();
    InitDeco3d(0);
    InitEffectOnContact();
    InitMovieDeco();
    bool reg = !(m_ctorFlags & EGEConstructorFlagsDef::Interface) && !(m_ctorFlags & EGEConstructorFlagsDef::DontRegister);
    RegisterForUpdate((m_ctorFlags & 1) ? false : reg);
}

// @0x57f59c: a copy of the collision entity the data points at, linked to the object (it follows it), the size of it and
// this the listener of it
void Object::InitCollision3D() {
    const clara::Param *p = FindParamByName(k_objectCollision3d);
    if (!p) return;
    const clara::Path &path = p->GetAsPath(0);
    if (path.IsEmpty()) return;
    GameEntity *source = static_cast<GameEntity*>(clara::Project::GetInstance()->FindEntityByPath(path));
    m_collision = new Collision3d(source);
    m_collision->LinkTo(this, jet::String::null, 0);
    m_collision->SetPosition(source->GetPosition());
    m_collision->SetRotation(source->GetRotation());
    const math::vec3<float> s = source->GetScale(), mine = GetScale();
    m_collision->SetScale(math::vec3<float>(s.x * mine.x, s.y * mine.y, s.z * mine.z));
    m_collision->SetCollisionListener(this);
}

// @0x57faec: a minion touching the object makes the effects of the contact (the effects come with the effect manager)
void Object::CollidesWith(Collision3d *other, const CollisionPair&) {
    if (m_effectsOnContact.empty()) return;
    Collision3dListener *l = other->GetCollisionListener();
    if (!l || l->GetAsEntity()->GetType() != 9) return;
    SpawnEffects(m_effectsOnContact, !m_effectOnContactSpawnAll);
}

// @0x57f218: the movie linked deco has this object as its linked object
void Object::InitMovieDeco() {
    if (!FindParamByName(k_movieLinkedDeco)) return;
    clara::Path path;
    if (const clara::Param *p = FindParamByName(k_movieLinkedDeco)) path = p->GetAsPath(0);
    if (clara::DataEntity *e = clara::Project::GetInstance()->FindEntityByPath(path))
        if (Deco3d *d = dynamic_cast<Deco3d*>(e)) d->SetMovieLinkedObject(this);
}

// @0x57f2ac: the deco made for this object is replaced, keeping where it is relative to its linkee
void Object::AdoptDeco3d(Deco3d *d) {
    math::vec3<float> pos(0, 0, 0);
    math::quat<float> rot(0, 0, 0, 1);
    clara::Entity *linkee = this;
    if (m_deco) {
        if (m_deco->GetLinkee()) linkee = m_deco->GetLinkee();
        const clara::Entity::LinkData *l = m_deco->GetLinkData();
        if (l) { pos = l->position; rot = l->rotation; } else { pos = m_deco->GetPosition(); rot = m_deco->GetRotation(); }
    }
    // @0x57f2ac: the effects on the old model follow the new one before the old deco goes
    EffectMgr::GetInstance()->OnModelChanged(m_deco ? m_deco->GetModel() : 0, d->GetModel());
    DestroyDeco3d();
    m_deco = d;
    d->LinkTo(linkee, jet::String::null, 0);
    d->SetTransform(pos, rot);
    UpdateTransform(false);
}

// @0x57f3e4: a copy of the reference deco linked to this object, scaled by this object's scale
void Object::InitDeco3d(Deco3d *ref) {
    if (!ref) {
        ref = GetDecoReference(&m_decoIndex, false);
        m_decoReference = ref;
        if ((!ref || !ref->CanLoadModel()) && !(ref = m_defaultDeco)) return;
    }
    m_deco = new Deco3d(ref, m_ctorFlags);
    m_deco->LinkTo(this, jet::String::null, 0);
    UpdateTransform(false);
    m_deco->SetPosition(ref->GetPosition());
    m_deco->SetRotation(ref->GetRotation());
    math::vec3<float> a = ref->GetScale(), b = GetScale();
    m_deco->SetScale(math::vec3<float>(b.x * a.x, b.y * a.y, b.z * a.z));
}

// @0x57f540: the deco goes to the level's list of entities to delete
void Object::DestroyDeco3d() {
    GameLevel::GetInstance()->DeleteEntityLater(m_deco);
    m_deco->RegisterForRender(false);
    m_deco = 0;
}

// @0x57f6f0
void Object::InitEffectOnContact() {
    if (HasParam(k_effectOnContact)) {
        const clara::Param *p = FindParamByName(k_effectOnContact);
        for (unsigned i = 0; i < p->GetComponentCount(); i++) {
            const clara::Path &path = p->GetAsPath(i);
            if (!path.IsEmpty()) m_effectsOnContact.push_back(reinterpret_cast<const EffectDef*>(clara::Project::GetInstance()->FindEntityByPath(path)));
        }
    }
    if (HasParam(k_effectOnContactSpawnAll)) GetParam(k_effectOnContactSpawnAll, m_effectOnContactSpawnAll);
}

// @0x57f9d0: the sounds and particle systems attached to the entity are not ported yet
void Object::Update() { clara::Entity::Update(); }

// @0x57f9d4
Deco3d *Object::GetDecoReference(unsigned *index, bool onlyLoadable) const {
    if (!m_decoReference) {
        const clara::Param *p = FindParamByName(k_objectDeco);
        unsigned i = jet::core::Rand() % p->GetComponentCount();
        const clara::Path &path = p->GetAsPath(i);
        if (path.IsEmpty()) return onlyLoadable ? m_defaultDeco : 0;
        if (index) *index = i;
        Deco3d *d = static_cast<Deco3d*>(clara::Project::GetInstance()->FindEntityByPath(path));
        bool check = onlyLoadable && d;
        if (!check || d->CanLoadModel() || !m_defaultDeco) return d;
        return m_defaultDeco;
    }
    if (!onlyLoadable || m_decoReference->CanLoadModel() || !m_defaultDeco) return m_decoReference;
    return m_defaultDeco;
}

// @0x57fb70
void Object::NotifyAnimationChanged() {
    if (!m_deco) return;
    m_deco->SetAnimationName(GetAnimationName());
    m_deco->clara::Entity::SetAnimation(GetAnimationName(), GetAnimationParams());
}

// @0x57fbc0: a copy of the deco, linked to this object's deco (or to this object)
void Object::AddLinkedEntity(Deco3d *d) {
    // @0x57fbc0: the copy hangs on the collision (at 0xC4) when there is one, else on this object
    clara::Entity *to = m_collision ? static_cast<clara::Entity*>(m_collision) : static_cast<clara::Entity*>(this);
    Deco3d *copy = new Deco3d(d->GetTemplate());
    copy->ConstructFrom(static_cast<const GameEntity*>(d));
    copy->LinkTo(to, jet::String(), 0x1f);
    copy->SetPosition(d->GetPosition());
    copy->SetRotation(d->GetRotation());
    copy->SetScale(d->GetScale());
    m_linkedDecos.push_back(copy);
}

// @0x57f7d8
void Object::SpawnEffects(const std::vector<const EffectDef*> &effects, bool randomOne) {
    if (effects.empty()) return;
    math::vec3<float> pos = GetPosition();
    math::quat<float> rot = GetRotation();
    if (IsLinked() && dynamic_cast<Deco3d*>(GetLinkee())) {
        // the linked deco gives the link point's rotation; the place stays the object's
        GetLinkee()->GetLinkPointTransform(GetLinkPointName(), pos, rot);
        pos = GetPosition();
    }
    EffectMgr *mgr = EffectMgr::GetInstance();
    if (randomOne) {
        const unsigned index = jet::core::Rand() % effects.size();
        mgr->Add(m_effectLayerId, effects[index], pos, rot, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
        return;
    }
    for (size_t i = 0; i < effects.size(); i++)
        mgr->Add(m_effectLayerId, effects[i], pos, rot, math::vec3<float>(1, 1, 1), true, 0, -1, true, 0);
}

// @0x57fcd4
bool Object::GetLinkPointTransform(const jet::String &point, math::vec3<float> &pos, math::quat<float> &rot) const {
    if (!point.empty() && m_deco) return m_deco->GetLinkPointTransform(point, pos, rot);
    return clara::Entity::GetLinkPointTransform(jet::String::null, pos, rot);
}

// @0x57fd34: out of the scene when the deco and the linked deco are
bool Object::IsOutOfScene() const {
    if (!GetPendingToDelete()) return false;
    bool out = true;
    if (m_deco) {
        m_deco->SetPendingToDelete(m_distanceToDelete);
        out = m_deco->IsOutOfScene();
    }
    if (m_collision) {
        m_collision->SetPendingToDelete(m_distanceToDelete);
        out = m_collision->IsOutOfScene() && out;
    }
    return out;
}

// @0x57fdb4
void Object::SetPersistenceType(int t) { GameEntity::SetPersistenceType(t); }

// @0x57fdb8
bool Object::CanLoadModel() const {
    Deco3d *d = GetDecoReference(0, false);
    return d && d->CanLoadModel();
}

// @0x57fdd8: the collision of the object (placed by code it enters the world now), then the deco's
void Object::OnPositionedInWorld() {
    if (m_collision) m_collision->OnPositionedInWorld();
    if (m_deco) m_deco->OnPositionedInWorld();
}
