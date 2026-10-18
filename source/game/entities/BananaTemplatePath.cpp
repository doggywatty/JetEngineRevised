// source/game/entities/BananaTemplatePath.cpp
#include "BananaTemplatePath.h"
#include "BananaTemplate.h"
#include "../GameLevel.h"
#include "param.h"
#include "scene/scenemgr.h"
#include "physics/physicsworld.h"

jet::String BananaTemplatePath::k_tmplName("BananaTemplatePath");
const jet::String BananaTemplatePath::k_points("Points"), BananaTemplatePath::k_pathWidth("PathWidth"), BananaTemplatePath::k_interpType("InterpType"),
    BananaTemplatePath::k_numSamples("NumSamples"), BananaTemplatePath::k_tension("Tension"), BananaTemplatePath::k_bias("Bias"),
    BananaTemplatePath::k_numObjectsInPath("numObjectsInPath"), BananaTemplatePath::k_projectOnGround("ProjectOnGround");

// @0x5a0cc4
BananaTemplatePath::BananaTemplatePath(const clara::Template *t) : GameEntity(t), m_updateTransform(false), m_projectOnGround(false) {}

// @0x5a0d10: the bananas go with the path
BananaTemplatePath::~BananaTemplatePath() {
    for (BananaTemplate *b : m_bananas) delete b;
    RegisterForUpdate(false);
}

// @0x5a0dec
void BananaTemplatePath::Init() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::Init();
}

// @0x5a0dfc
void BananaTemplatePath::PostInit() {
    if (m_flags & F_LIBRARY) return;
    GameEntity::PostInit();
    GetParam(k_projectOnGround, m_projectOnGround);
    BuildPath();
    CreateBananas();
    RegisterForUpdate(true);
}

// @0x5a0e60: the bananas are put on the ground once that works
void BananaTemplatePath::Update() {
    if (!m_updateTransform) return;
    if (RefreshBananaTransform()) m_updateTransform = false;
}

// @0x5a1448
void BananaTemplatePath::NotifyTransformChanged(bool b) {
    GameEntity::NotifyTransformChanged(b);
    if (m_projectOnGround) m_updateTransform = true;
}

// @0x5a0e88: the path through the entities of the points (a single point gets another one in front of it)
void BananaTemplatePath::BuildPath() {
    if (m_path) return;
    const clara::Param *points = FindParamByName(k_points);
    std::vector<math::vec3<float> > positions;
    std::vector<math::quat<float> > rotations;
    std::vector<float> widths;
    float width = 0;
    int interpType = 0;
    int samples = 0;
    float tension = 0, bias = 0;
    GetParam(k_pathWidth, width);
    GetParam(k_interpType, interpType);
    GetParam(k_numSamples, samples);
    GetParam(k_tension, tension);
    GetParam(k_bias, bias);
    if (!points) return;
    for (unsigned i = 0; i < points->GetComponentCount(); i++) {
        clara::Entity *e = dynamic_cast<clara::Entity*>(points->GetAsEntity(i));
        positions.push_back(e->GetPosition());
        rotations.push_back(e->GetRotation());
        widths.push_back(width);
    }
    if (positions.size() < 2) positions.push_back(positions.front() + jet::scene::SceneMgr::GetFrontVector());
    m_path.reset(new PathCommon(positions, rotations, widths, width, jet::core::InterpolationType((unsigned char)interpType), (unsigned)samples, tension, bias,
                                jet::video::Color(0x5555ff55u)));
}

// @0x5a115c (the .so's symbol is spelled ProjectOnGrund): the ground under a banana, by a ray from 50 m above it to 50 m below it through the
// level's physics world (group 5, mask 2), the hit point replaces the position
bool BananaTemplatePath::ProjectOnGround(math::vec3<float> &pos) {
    PhysicsWorld *world = GameLevel::GetInstance()->GetPhysicsWorld();
    if (!world) return false;
    static RaycastResult hit;
    const math::vec3<float> reach = jet::scene::SceneMgr::GetUpVector() * 50.0f;
    if (!world->RayCast(pos + reach, pos - reach, 5, 2, hit)) return false;
    pos = hit.position;
    return true;
}

// @0x5a12d8
unsigned BananaTemplatePath::GetNumObjectsToCreate() {
    int n = 0;
    GetParam(k_numObjectsInPath, n);
    return (unsigned)n;
}

// @0x5a1308: the bananas, linked to the path
void BananaTemplatePath::CreateBananas() {
    if (!m_path) return;
    const unsigned n = GetNumObjectsToCreate();
    for (unsigned i = 0; i < n; i++) {
        BananaTemplate *b = new BananaTemplate(0);
        b->ConstructFrom(static_cast<const GameEntity*>(this));
        b->LinkTo(this, jet::String::null, 0);
        b->SetPosition(math::vec3<float>(0, 0, 0));
        b->SetRotation(math::quat<float>(0, 0, 0, 1));
        b->SetPersistenceType(GameEntity::PersistenceTypeDef::Default);
        m_bananas.push_back(b);
    }
    RefreshBananaTransform();
}

// @0x5a1468: the bananas are spread along the path (the last on its end); on the ground when asked, false while that cannot be done
bool BananaTemplatePath::RefreshBananaTransform() {
    if (!m_path) return true;
    const unsigned last = (unsigned)m_path->GetSegmentLenghts().size() - 1;
    const size_t count = m_bananas.size();
    float step = m_path->GetTravelDistanceTo(last, 1.0f);
    if (count > 1) step /= (float)(count - 1);
    bool done = true;
    unsigned segment = 0;
    float t = 0;
    for (size_t i = 0; i < count; i++) {
        if (i == count - 1) {
            segment = last;
            t = 1.0f;
        }
        const math::vec3<float> position = m_path->GetPositionAt(segment, t);
        const math::quat<float> rotation = m_path->GetRotationAt(segment, t);
        m_path->TravelAlongPath(segment, t, step, segment, t, 0);
        BananaTemplate *b = m_bananas[i];
        b->SetPosition(position);
        b->SetRotation(rotation);
        if (!m_projectOnGround) continue;
        math::vec3<float> ground = b->GetPosition();
        if (!ProjectOnGround(ground)) {
            done = false;
            continue;
        }
        const clara::Entity::LinkData *l = b->GetLinkData();
        b->SetPosition((l ? l->position : b->clara::Entity::GetPosition()) + (ground - b->GetPosition()));
    }
    return done;
}

// @0x5a16ac
void BananaTemplatePath::OnLinkedHidden() {
    for (BananaTemplate *b : m_bananas) b->OnLinkedHidden();
}
