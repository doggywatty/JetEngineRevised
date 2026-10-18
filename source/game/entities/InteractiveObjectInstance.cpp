// source/game/entities/InteractiveObjectInstance.cpp
#include "InteractiveObjectInstance.h"
#include "InteractiveObjectTemplate.h"
#include "param.h"
#include "project.h"

jet::String InteractiveObjectInstance::k_tmplName("InteractiveObjectInstance");
const jet::String InteractiveObjectInstance::k_template("Template");

// @0x56c70c
InteractiveObjectInstance::InteractiveObjectInstance(const clara::Template *t) : GameEntity(t), m_createdObject(0), m_keepCreatedEntity(false) {
    m_type = 0xe;
}

// @0x56c750: the object goes with it, unless it was kept
InteractiveObjectInstance::~InteractiveObjectInstance() {
    if (!m_keepCreatedEntity) delete m_createdObject;
}

Object *InteractiveObjectInstance::GetCreatedEntity() const { return m_createdObject; }

// @0x56c5fc: the place of the object it made
const math::vec3<float> &InteractiveObjectInstance::GetPosition() const { return m_createdObject ? m_createdObject->GetPosition() : GameEntity::GetPosition(); }

// @0x56c628
void InteractiveObjectInstance::SetPosition(const math::vec3<float> &p) {
    if (m_createdObject) m_createdObject->SetPosition(p);
    clara::Entity::SetPosition(p);
}

// @0x56c65c
const math::quat<float> &InteractiveObjectInstance::GetRotation() const { return m_createdObject ? m_createdObject->GetRotation() : GameEntity::GetRotation(); }

// @0x56c688
void InteractiveObjectInstance::SetRotation(const math::quat<float> &r) {
    if (m_createdObject) m_createdObject->SetRotation(r);
    clara::Entity::SetRotation(r);
}

// @0x56c6bc
void InteractiveObjectInstance::Translate(const math::vec3<float> &d) {
    if (m_createdObject) m_createdObject->Translate(d);
}

// @0x56c6dc
void InteractiveObjectInstance::SetPersistenceType(int t) {
    GameEntity::SetPersistenceType(t);
    if (m_createdObject) m_createdObject->SetPersistenceType(t);
}

// @0x56c7d8
void InteractiveObjectInstance::Init() {
    if (IsLibraryEntity()) return;
    GameEntity::Init();
}

// @0x56c7e8: a copy of the object the "Template" parameter names is made at the place of this instance, put on top of the object's
void InteractiveObjectInstance::PostInit() {
    if (IsLibraryEntity()) return;
    const clara::Path &path = FindParamByName(k_template)->GetAsPath(0);
    clara::Project *project = clara::Project::GetInstance();
    m_createdObject = new InteractiveObjectTemplate(project->FindTemplateByName(InteractiveObjectTemplate::k_tmplName));
    clara::Entity *ref = static_cast<clara::Entity*>(project->FindEntityByPath(path));
    const math::vec3<float> position = ref->GetPosition();
    const math::quat<float> rotation = ref->GetRotation();
    const math::vec3<float> scale = ref->GetScale();
    ref->SetPosition(position + GetPosition());
    ref->SetRotation(rotation * GetRotation());
    const math::vec3<float> mine = GetScale();
    ref->SetScale(math::vec3<float>(scale.x * mine.x, scale.y * mine.y, scale.z * mine.z));
    m_createdObject->ConstructFrom(dynamic_cast<const GameEntity*>(ref));
    ref->SetPosition(position);
    ref->SetRotation(rotation);
    ref->SetScale(scale);
    RegisterForUpdate(true);
    RegisterForRender(false);
}
