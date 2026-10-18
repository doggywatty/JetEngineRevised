// core/claraloader/include/entity.h
#ifndef CORE_CLARALOADER_INCLUDE_ENTITY_H
#define CORE_CLARALOADER_INCLUDE_ENTITY_H

#include <memory>
#include "jrtti.h"
#include "mem/memorymanager.h"
#include "param.h"
#include "clarafile.h"

namespace clara {

// A node of a clara file: a template instance with its parameter values.
class DataEntity {
    JET_RTTI_ROOT(DataEntity)
public:
    enum Flags { F_INIT = 1, F_POSTINIT = 2, F_ENABLED = 4, F_VISIBLE = 8, F_MOVIE = 0x10, F_LIBRARY = 0x20, F_SEARCHABLE = 0x40 };

    DataEntity();
    explicit DataEntity(const Template* t);
    DataEntity(const DataEntity&) = delete;
    DataEntity& operator=(const DataEntity&) = delete;
    virtual ~DataEntity() {}

    virtual void Init();
    virtual void PostInit();
    virtual void ConstructFrom(const DataEntity* o);
    virtual void SetEnabled(bool on) { SetFlag(F_ENABLED, on); }
    virtual void SetVisible(bool on) { SetFlag(F_VISIBLE, on); }
    virtual void SetSearchable(bool on) { SetFlag(F_SEARCHABLE, on); }
    virtual void Load(jet::stream::IStream& s, Project& p);
    virtual void Unload() { m_params.reset(); }
    virtual void Serialize(jet::stream::IStreamW&) {}
    virtual void Deserialize(jet::stream::IStream&) {}
    virtual const jet::String& GetBaseTemplateName() const { return GetTemplateName(); }

    const jet::String& GetName() const { return m_name; }
    void SetName(const jet::String& n) { m_name = n; }
    const Template* GetTemplate() const { return m_template; }
    const jet::String& GetTemplateName() const;
    unsigned GetId() const { return m_id; }
    bool IsEnabled() const { return (m_flags & F_ENABLED) != 0; }
    bool IsVisible() const { return (m_flags & F_VISIBLE) != 0; }
    bool IsSearchable() const { return (m_flags & F_SEARCHABLE) != 0; }
    ClaraFile* GetClaraFile() const { return m_file; }
    // entities of a library file are templates for the copies the game places
    void SetClaraFile(ClaraFile* f) { m_file = f; SetFlag(F_LIBRARY, f && f->IsLibrary()); }
    bool IsLibraryEntity() const { return (m_flags & F_LIBRARY) != 0; }

    unsigned GetParamCount() const { return m_template && m_params ? m_template->GetParamCount() : 0; }
    Param* GetParam(unsigned i) const { return m_params.get() + i; }
    Param* FindParamByName(const jet::String& name) const;
    bool HasParam(const jet::String& name) const { return FindParamByName(name) != nullptr; }
    bool GetParam(const jet::String& name, float& v, unsigned i = 0) const;
    bool GetParam(const jet::String& name, int& v, unsigned i = 0) const;
    bool GetParam(const jet::String& name, bool& v, unsigned i = 0) const;
    bool GetParam(const jet::String& name, jet::String& v, unsigned i = 0) const;
    bool GetParam(const jet::String& name, math::vec2<float>& v, unsigned i = 0) const;
    bool GetParam(const jet::String& name, math::vec3<float>& v, unsigned i = 0) const;
    bool GetParam(const jet::String& name, math::vec4<float>& v, unsigned i = 0) const;
    // "param/component/param/..." into nested entity parameters.
    DataEntity* FindComponentByRelativePath(const Path& p, unsigned start = 0) const;

protected:
    friend class Folder;
    friend class Project;
    friend class Param;
    void SetFlag(unsigned f, bool on) { m_flags = on ? (m_flags | f) : (m_flags & ~f); }
    void LoadParams(jet::stream::IStream& s, Project& p);

    jet::String m_name;
    const Template* m_template;
    ClaraFile* m_file;
    unsigned m_id;
    unsigned m_flags;
    std::shared_ptr<Param> m_params;
};

// A placed entity: transform, optional animation and attachment to another entity (Entity, clara.so
// @0x615374). Linked, its Set* change the offset to the link and UpdateTransform computes the world
// transform from the linkee's link point.
class Entity : public DataEntity {
    JET_RTTI(Entity, DataEntity)
public:
    struct AnimationParams {  // 0x10: the entity's animation start time, current time, loop count and loop flag
        unsigned startTime = 0;
        unsigned time = 0;
        int loops = -1;
        bool loop = false;
    };
    // Which links UpdateLinkedEntities reaches: those on the entity itself, on one of its link points, all
    struct LinkedToDef { enum type { Entity = 0, LinkPoint = 1, All = 2 }; };
    // LinkData flags: bit 0 aligns (follows the linkee's movement) instead of attaching; 2, 4, 8 the axes an
    // aligned entity follows; 0x10 moves in the linkee's frame; 0x20 follows its rotation
    struct LinkFlags { enum { Align = 1, AxisX = 2, AxisY = 4, AxisZ = 8, WorldDelta = 0x10, FollowRotation = 0x20 }; };
    struct LinkData {
        Path path;
        jet::String linkPoint;                 // 0x20
        unsigned flags = 0;                    // 0x24
        Entity* linkee = nullptr;              // 0x1c
        math::vec3<float> axisMask;            // 0x28
        math::vec3<float> lastLinkPos;         // 0x34
        math::vec3<float> accum;               // 0x40
        math::vec3<float> position;            // 0x4c offset to the link point
        math::quat<float> rotation;            // 0x58
    };
    struct AnimationData {
        jet::String name;
        AnimationParams params;
        bool controlledByMovie = false;
    };

    Entity();
    explicit Entity(const Template* t);
    virtual ~Entity();
    virtual void Load(jet::stream::IStream& s, Project& p);
    virtual void Init();
    virtual void PostInit();
    virtual void Update() {}
    virtual const math::vec3<float>& GetPosition() const { return m_position; }
    virtual void SetPosition(const math::vec3<float>& v) { if (SetPositionIfChanged(v)) UpdateTransform(false); }
    virtual const math::quat<float>& GetRotation() const { return m_rotation; }
    virtual void SetRotation(const math::quat<float>& q) { if (SetRotationIfChanged(q)) UpdateTransform(false); }
    virtual const math::vec3<float>& GetScale() const { return m_scale; }
    virtual void SetScale(const math::vec3<float>& v) { if (SetScaleIfChanged(v)) NotifyTransformChanged(false); }
    virtual void ConstructFrom(const Entity* o);
    virtual void NotifyParamChanged(const jet::String&) {}
    virtual void NotifyAnimationChanged() {}
    virtual void NotifyMovieStatusChanged() {}
    virtual void NotifyTransformChanged(bool) {}
    virtual bool GetLinkPointTransform(const jet::String&, math::vec3<float>& pos, math::quat<float>& rot) const;

    void SetTransform(const math::vec3<float>& p, const math::quat<float>& r);
    void SetTransform(const math::vec3<float>& p, const math::quat<float>& r, const math::vec3<float>& s);
    bool SetPositionIfChanged(const math::vec3<float>& v);
    bool SetRotationIfChanged(const math::quat<float>& q);
    bool SetScaleIfChanged(const math::vec3<float>& s);
    // the position / rotation a world one has relative to the link point
    math::vec3<float> ToLocalPosition(const math::vec3<float>&) const;
    math::quat<float> ToLocalRotation(const math::quat<float>&) const;
    math::mat4<float> GetLocalTransform() const;
    math::mat4<float> GetTransform() const;
    // recompute the world transform from the link, tell the entity, then (unless fromParent) the entities linked to it
    void UpdateTransform(bool fromParent);
    void UpdateLinkedEntities(int linkedTo);

    const AnimationParams& GetAnimationParams() const;
    const AnimationData* GetAnimation() const { return m_anim.get(); }
    const jet::String& GetAnimationName() const;
    void SetAnimation(const jet::String& name);
    void SetAnimation(const jet::String& name, const AnimationParams& params);
    bool IsLinked() const { return m_link != nullptr && m_link->linkee != nullptr; }
    bool IsAttached() const { return IsLinked() && !(m_link->flags & LinkFlags::Align); }
    bool IsAligned() const { return IsLinked() && (m_link->flags & LinkFlags::Align); }
    const LinkData* GetLinkData() const { return m_link.get(); }
    Entity* GetLinkee() const { return IsLinked() ? m_link->linkee : nullptr; }
    const jet::String& GetLinkPointName() const;
    void LinkTo(Entity* e, const jet::String& linkPoint, unsigned flags = 0);
    bool IsLinkedTo(Entity* e) const { return IsLinked() && m_link->linkee == e; }
    bool IsLinkedTo(Entity* e, const jet::String& point) const { return IsLinkedTo(e) && m_link->linkPoint.Equals(point); }
    unsigned GetLinkedEntitiesCount() const { return m_linked ? unsigned(m_linked->size()) : 0; }
    Entity* GetLinkedEntity(unsigned i) const { return (*m_linked)[i].first; }
    bool IsControlledByMovie() const { return m_anim && m_anim->controlledByMovie; }
    void SetControlledByMovie(bool on) { if (m_anim) m_anim->controlledByMovie = on; }
    bool IsMovieEnabled() const { return (m_flags & F_MOVIE) != 0; }
    void SetMovieEnabled(bool on) { SetFlag(F_MOVIE, on); }

private:
    void UpdateLinkAlign();
    void UpdateLinkAttach();
    void AddLinkedEntity(Entity* e, const jet::String& linkPoint);
    void RemoveLinkedEntity(Entity* e);

    std::unique_ptr<LinkData> m_link;      // 0x24
    math::vec3<float> m_position;          // 0x28
    math::quat<float> m_rotation;          // 0x34
    math::vec3<float> m_scale;             // 0x44
    std::unique_ptr<AnimationData> m_anim; // 0x50
    typedef std::vector<std::pair<Entity*, int> > LinkedEntities;
    std::unique_ptr<LinkedEntities> m_linked;  // 0x20: entity, LinkedToDef
};

// Makes the objects for template instances; the game supplies its own to get its entity classes.
class Creator {
public:
    virtual ~Creator() {}
    virtual DataEntity* CreateEntity(const Template* t) const = 0;
};

class DefaultCreator : public Creator {
public:
    virtual DataEntity* CreateEntity(const Template* t) const { return new Entity(t); }
};

}

#endif
