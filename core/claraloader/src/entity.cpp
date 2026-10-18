// core/claraloader/src/entity.cpp
#include "entity.h"
#include <algorithm>
#include "project.h"

namespace clara {

static unsigned NewId() { return Project::GetInstance() ? Project::GetInstance()->GetNewId() : 0; }

DataEntity::DataEntity() : m_template(nullptr), m_file(nullptr), m_id(NewId()), m_flags(F_SEARCHABLE) {}

DataEntity::DataEntity(const Template* t) : m_template(t), m_file(nullptr), m_id(NewId()), m_flags(F_SEARCHABLE) {}

const jet::String& DataEntity::GetTemplateName() const { return m_template ? m_template->GetName() : jet::String::null; }

void DataEntity::LoadParams(jet::stream::IStream& s, Project& p)
{
    u16 n = 0;
    s >> n;
    if (!n || !m_template) {
        m_params.reset();
        return;
    }
    m_params.reset(new Param[n], std::default_delete<Param[]>());
    for (unsigned i = 0; i < n; ++i) {
        m_params.get()[i].SetParamDef(m_template->GetParam(i));
        m_params.get()[i].Load(s, p);
    }
}

// Same stream layout as Entity::Load; the transform, animation and link fields are read and dropped.
void DataEntity::Load(jet::stream::IStream& s, Project& p)
{
    jet::mem::Tag tag("clara");
    math::vec3<float> pos, scale;
    math::quat<float> rot;
    jet::String anim;
    s >> m_name >> pos >> rot >> scale >> anim;
    if (!anim.empty()) {
        u32 a, b;
        bool c;
        s >> a >> b >> c;
    }
    Path link;
    s >> link;
    if (!link.IsEmpty()) {
        jet::String lp;
        u32 f;
        s >> lp >> f;
    }
    LoadParams(s, p);
}

void DataEntity::Init()
{
    if (m_flags & F_INIT)
        return;
    for (unsigned i = 0; i < GetParamCount(); ++i)
        GetParam(i)->Init();
    m_flags |= F_INIT;
}

void DataEntity::PostInit()
{
    if (m_flags & F_POSTINIT)
        return;
    for (unsigned i = 0; i < GetParamCount(); ++i)
        GetParam(i)->PostInit();
    m_flags |= F_POSTINIT;
}

void DataEntity::ConstructFrom(const DataEntity* o)
{
    m_template = o->m_template;
    m_params = o->m_params;
    m_file = o->m_file;
    // a copy is made ready at once
    Init();
    PostInit();
}

Param* DataEntity::FindParamByName(const jet::String& name) const
{
    for (unsigned i = 0; i < GetParamCount(); ++i)
        if (GetParam(i)->GetName() == name)
            return GetParam(i);
    return nullptr;
}

bool DataEntity::GetParam(const jet::String& name, float& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetAsFloat(i);
    return p != nullptr;
}

bool DataEntity::GetParam(const jet::String& name, int& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetAsS32(i);
    return p != nullptr;
}

bool DataEntity::GetParam(const jet::String& name, bool& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetAsU32(i) != 0;
    return p != nullptr;
}

bool DataEntity::GetParam(const jet::String& name, jet::String& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetType() && p->GetType()->GetValueType() == Type::ENUM ? p->GetAsEnumName(i) : p->GetAsString(i);
    return p != nullptr;
}

bool DataEntity::GetParam(const jet::String& name, math::vec2<float>& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetAsVector2D(i);
    return p != nullptr;
}

bool DataEntity::GetParam(const jet::String& name, math::vec3<float>& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetAsVector3D(i);
    return p != nullptr;
}

bool DataEntity::GetParam(const jet::String& name, math::vec4<float>& v, unsigned i) const
{
    Param* p = FindParamByName(name);
    if (p)
        v = p->GetAsVector4D(i);
    return p != nullptr;
}

DataEntity* DataEntity::FindComponentByRelativePath(const Path& path, unsigned start) const
{
    const DataEntity* e = this;
    for (unsigned i = start; e && i < path.GetCount(); i += 2) {
        Param* p = e->FindParamByName(path.Get(int(i)));
        if (!p)
            return nullptr;
        unsigned c = 0;
        if (i + 1 < path.GetCount()) {
            const jet::String& cn = path.Get(int(i + 1));
            for (c = 0; c < p->GetComponentCount() && p->GetComponentName(c) != cn; ++c) {
            }
        }
        e = p->GetAsEntity(c);
    }
    return const_cast<DataEntity*>(e);
}

Entity::Entity() : m_rotation(0, 0, 0, 1), m_scale(1, 1, 1) { m_flags |= F_ENABLED | F_VISIBLE | F_MOVIE; }

Entity::Entity(const Template* t) : DataEntity(t), m_rotation(0, 0, 0, 1), m_scale(1, 1, 1)
{
    m_flags |= F_ENABLED | F_VISIBLE | F_MOVIE;
}

Entity::~Entity()
{
    if (m_link && m_link->linkee)
        m_link->linkee->RemoveLinkedEntity(this);
    if (m_linked)
        for (auto& e : *m_linked)
            if (e.first->m_link)
                e.first->m_link->linkee = nullptr;
}

// name, vec3 position, quat rotation, vec3 scale,
// anim name [u32 start, s32 loops, bool playing], link path [link point, u32 flags], params.
void Entity::Load(jet::stream::IStream& s, Project& p)
{
    jet::mem::Tag tag("clara");
    jet::String anim;
    s >> m_name >> m_position >> m_rotation >> m_scale >> anim;
    if (!anim.empty()) {
        m_anim.reset(new AnimationData);
        m_anim->name = anim;
        s >> m_anim->params.startTime >> m_anim->params.loops >> m_anim->params.loop;
        m_anim->params.time = m_anim->params.startTime;
    }
    Path link;
    s >> link;
    if (!link.IsEmpty()) {
        m_link.reset(new LinkData);
        m_link->path = link;
        m_link->position = m_position;
        m_link->rotation = m_rotation;
        s >> m_link->linkPoint >> m_link->flags;
    }
    LoadParams(s, p);
}

// the link is resolved and the world transform computed from it
void Entity::Init()
{
    if (m_flags & F_INIT)
        return;
    DataEntity::Init();
    if (m_link && !m_link->path.IsEmpty() && Project::GetInstance()) {
        DataEntity* e = m_file ? Project::GetInstance()->FindEntityByPath(*m_file, m_link->path) : Project::GetInstance()->FindEntityByPath(m_link->path);
        Entity* linkee = e && e->RttiIsOfType(Entity::RttiGetClassId()) ? static_cast<Entity*>(e) : nullptr;
        m_link->path.Clear();
        LinkTo(linkee, m_link->linkPoint, m_link->flags);
        if (m_link) {
            if (!(m_link->flags & LinkFlags::Align))
                UpdateLinkAttach();
            else
                UpdateLinkAlign();
        }
    }
    if (m_link && !m_link->linkee)
        m_link.reset();
}

void Entity::PostInit()
{
    if (m_flags & F_POSTINIT)
        return;
    UpdateTransform(false);
    DataEntity::PostInit();
    if (!GetAnimationName().empty())
        NotifyAnimationChanged();
}

void Entity::ConstructFrom(const Entity* o)
{
    m_name = jet::String::Format("%s_%u", o->m_name.c_str(), m_id);
    m_position = o->GetPosition();
    m_rotation = o->GetRotation();
    m_scale = o->GetScale();
    DataEntity::ConstructFrom(o);
}

bool Entity::GetLinkPointTransform(const jet::String& name, math::vec3<float>& pos, math::quat<float>& rot) const
{
    if (!name.empty())
        return false;
    pos = GetPosition();
    rot = GetRotation();
    return true;
}

bool Entity::SetPositionIfChanged(const math::vec3<float>& v)
{
    math::vec3<float>& dst = m_link ? m_link->position : m_position;
    if (math::equals(v, dst, 1.1920929e-07f))
        return false;
    dst = v;
    return true;
}

bool Entity::SetRotationIfChanged(const math::quat<float>& q)
{
    math::quat<float>& dst = m_link ? m_link->rotation : m_rotation;
    if (math::equals(q, dst, 1.1920929e-07f))
        return false;
    dst = q;
    return true;
}

bool Entity::SetScaleIfChanged(const math::vec3<float>& s)
{
    if (math::equals(m_scale, s, 1.1920929e-07f))
        return false;
    m_scale = s;
    return true;
}

math::vec3<float> Entity::ToLocalPosition(const math::vec3<float>& p) const
{
    if (IsLinked()) {
        math::vec3<float> lp(0, 0, 0);
        math::quat<float> lr(0, 0, 0, 1);
        if (m_link->linkee->GetLinkPointTransform(m_link->linkPoint, lp, lr)) {
            math::mat4<float> m(lr.getAsMat3());
            m.setTranslation(lp);
            m.invert();
            return math::transform(m, p);
        }
    }
    return p;
}

math::quat<float> Entity::ToLocalRotation(const math::quat<float>& q) const
{
    if (IsLinked()) {
        math::vec3<float> lp(0, 0, 0);
        math::quat<float> lr(0, 0, 0, 1);
        if (m_link->linkee->GetLinkPointTransform(m_link->linkPoint, lp, lr))
            return math::quat<float>(-lr.x, -lr.y, -lr.z, lr.w) * q;
    }
    return q;
}

void Entity::SetTransform(const math::vec3<float>& p, const math::quat<float>& r)
{
    bool a = SetPositionIfChanged(p);
    bool b = SetRotationIfChanged(r);
    if (a || b)
        UpdateTransform(false);
}

void Entity::SetTransform(const math::vec3<float>& p, const math::quat<float>& r, const math::vec3<float>& s)
{
    bool a = SetPositionIfChanged(p);
    bool b = SetRotationIfChanged(r);
    bool c = SetScaleIfChanged(s);
    if (a || b || c)
        UpdateTransform(false);
}

math::mat4<float> Entity::GetLocalTransform() const
{
    math::mat4<float> m(m_link ? m_link->rotation.getAsMat3() : m_rotation.getAsMat3());
    m.postScale(m_scale);
    m.setTranslation(m_link ? m_link->position : m_position);
    return m;
}

math::mat4<float> Entity::GetTransform() const
{
    math::mat4<float> m(GetRotation().getAsMat3());
    m.postScale(GetScale());
    m.setTranslation(GetPosition());
    return m;
}

void Entity::UpdateTransform(bool fromParent)
{
    if (m_link) {
        if (!(m_link->flags & LinkFlags::Align))
            UpdateLinkAttach();
        else
            UpdateLinkAlign();
    }
    NotifyTransformChanged(fromParent);
    if (!fromParent)
        UpdateLinkedEntities(LinkedToDef::All);
}

void Entity::UpdateLinkedEntities(int linkedTo)
{
    if (!m_linked)
        return;
    for (size_t i = 0; i < m_linked->size(); ++i) {
        std::pair<Entity*, int> e = (*m_linked)[i];
        if (linkedTo == LinkedToDef::All || e.second == linkedTo)
            e.first->UpdateTransform(false);
    }
}

// Attached: the offset to the link point, in the link point's frame
void Entity::UpdateLinkAttach()
{
    math::vec3<float> lp(0, 0, 0);
    math::quat<float> lr(0, 0, 0, 1);
    if (!m_link->linkee->GetLinkPointTransform(m_link->linkPoint, lp, lr))
        return;
    m_position = lr * m_link->position + lp;
    m_rotation = lr * m_link->rotation;
}

// Aligned: only the link point's movement since the last update is followed, on the flagged axes
void Entity::UpdateLinkAlign()
{
    math::vec3<float> lp(0, 0, 0);
    math::quat<float> lr(0, 0, 0, 1);
    if (!m_link->linkee->GetLinkPointTransform(m_link->linkPoint, lp, lr))
        return;
    LinkData& l = *m_link;
    math::vec3<float> d = lp - l.lastLinkPos;
    l.lastLinkPos = lp;
    if (!(l.flags & LinkFlags::WorldDelta)) {
        math::vec3<float> loc = math::quat<float>(-lr.x, -lr.y, -lr.z, lr.w) * d;
        d = l.rotation * math::vec3<float>(l.axisMask.x * loc.x, l.axisMask.y * loc.y, l.axisMask.z * loc.z);
    } else {
        d = math::vec3<float>(d.x * l.axisMask.x, d.y * l.axisMask.y, d.z * l.axisMask.z);
    }
    l.accum = l.accum + d;
    m_position = l.position + l.accum;
    m_rotation = (l.flags & LinkFlags::FollowRotation) ? lr * l.rotation : l.rotation;
}

const jet::String& Entity::GetAnimationName() const { return m_anim ? m_anim->name : jet::String::null; }

const Entity::AnimationParams& Entity::GetAnimationParams() const
{
    static const AnimationParams none;
    return m_anim ? m_anim->params : none;
}

void Entity::SetAnimation(const jet::String& name) { SetAnimation(name, AnimationParams()); }

void Entity::SetAnimation(const jet::String& name, const AnimationParams& params)
{
    const AnimationParams& cur = GetAnimationParams();
    if (GetAnimationName().Equals(name) && cur.startTime == params.startTime && cur.time == params.time && cur.loops == params.loops && cur.loop == params.loop)
        return;
    if (!m_anim)
        m_anim.reset(new AnimationData);
    m_anim->name = name;
    m_anim->params = params;
    NotifyAnimationChanged();
}

const jet::String& Entity::GetLinkPointName() const { return m_link ? m_link->linkPoint : jet::String::null; }

// LinkTo @0x616694: the axis mask comes from the flags, the last link point and accumulated movement restart
void Entity::LinkTo(Entity* e, const jet::String& linkPoint, unsigned flags)
{
    if (m_link && m_link->linkee)
        m_link->linkee->RemoveLinkedEntity(this);
    if (!e) {
        m_link.reset();
        return;
    }
    if (!m_link) {
        m_link.reset(new LinkData);
        m_link->rotation = math::quat<float>(0, 0, 0, 1);
    }
    m_link->flags = flags;
    m_link->linkPoint = linkPoint;
    m_link->linkee = e;
    m_link->axisMask = math::vec3<float>((flags & LinkFlags::AxisX) ? 1.f : 0.f, (flags & LinkFlags::AxisY) ? 1.f : 0.f, (flags & LinkFlags::AxisZ) ? 1.f : 0.f);
    m_link->lastLinkPos = e->GetPosition();
    m_link->accum = e->GetPosition();
    e->AddLinkedEntity(this, linkPoint);
}

void Entity::AddLinkedEntity(Entity* e, const jet::String& linkPoint)
{
    if (!e)
        return;
    RemoveLinkedEntity(e);
    if (!m_linked)
        m_linked.reset(new LinkedEntities);
    m_linked->push_back(std::make_pair(e, linkPoint.empty() ? 0 : 1));
}

void Entity::RemoveLinkedEntity(Entity* e)
{
    if (!e || !m_linked)
        return;
    for (size_t i = 0; i < m_linked->size(); ++i)
        if ((*m_linked)[i].first == e) {
            m_linked->erase(m_linked->begin() + i);
            return;
        }
}

}
