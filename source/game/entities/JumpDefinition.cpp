// source/game/entities/JumpDefinition.cpp
#include "JumpDefinition.h"
#include "scene/scenemgr.h"

jet::String JumpDefinition::k_tmplName("JumpDefinition");
jet::String JumpDefinition::k_JumpPathPoint_tmplName("JumpPathPoint");

// @0x4e1bfc
JumpDefinition::JumpDefinition(const clara::Template *t)
    : GameEntity(t), m_interpolationType(0), m_totalTime(0), m_timeMultMin(0.95f), m_timeMultMax(1.15f), m_canFallOut(false) {}
JumpDefinition::~JumpDefinition() {}

// @0x4e1cf8
void JumpDefinition::Init() {
    if (!(m_flags & F_INIT)) clara::Entity::Init();
}

// @0x4e1cfc
void JumpDefinition::PostInit() {
    GameEntity::PostInit();
    BuildPath();
    GetParam(jet::String("totalTime"), m_totalTime);
    GetParam(jet::String("interpolationType"), m_interpolationType);
    GetParam(jet::String("MinTimeMult"), m_timeMultMin);
    GetParam(jet::String("MaxTimeMult"), m_timeMultMax);
    m_timeMultMin *= 0.01f;
    m_timeMultMax *= 0.01f;
    GetParam(jet::String("canFallOut"), m_canFallOut);
    RegisterForUpdate(false);
    RegisterForRender(false);
}

// @0x4e1df0: the points of the entity (in its own rotation) make the path; each point's "timePgt" says how much of the time has passed
// there, and the distance along the path at it is kept to follow it
void JumpDefinition::BuildPath() {
    if (m_path) return;
    const math::quat<float> rot = GetRotation();
    const clara::Param *points = FindParamByName(jet::String("Points"));
    std::vector<math::vec3<float> > positions;
    std::vector<math::quat<float> > rotations;
    std::vector<float> widths;
    float width = 0, tension = 0, bias = 0;
    int interp = 0;
    signed char samples = 0;
    GetParam(jet::String("PathWidth"), width);
    GetParam(jet::String("InterpType"), interp);
    int s = 0;
    GetParam(jet::String("NumSamples"), s);
    samples = (signed char)s;
    GetParam(jet::String("Tension"), tension);
    GetParam(jet::String("Bias"), bias);
    if (!points) return;
    float timePgt = 0;
    for (unsigned i = 0; i < points->GetComponentCount(); i++) {
        clara::DataEntity *p = points->GetAsEntity(i);
        positions.push_back(rot * static_cast<GameEntity*>(p)->GetPosition());
        rotations.push_back(rot * static_cast<GameEntity*>(p)->GetRotation());
        widths.push_back(width);
        p->GetParam(jet::String("timePgt"), timePgt);
        timePgt *= 0.01f;
        m_distanceDistribution.push_back({ timePgt, 0.0f });
    }
    if (positions.size() < 6) positions.push_back(positions.back() + jet::scene::SceneMgr::GetFrontVector());
    m_path.reset(new PathCommon(positions, rotations, widths, width, jet::core::InterpolationType((unsigned char)interp), samples, tension, bias, jet::video::Color(0x5555ff55)));
    const float total = m_path->GetTotalDistance();
    const unsigned perSegment = m_path->GetNumSamplesPerSegment() + 1;
    unsigned node = 0;
    for (unsigned i = 0; i < m_distanceDistribution.size(); i++, node += perSegment)
        m_distanceDistribution[i].distancePgt = m_path->GetTravelDistanceTo(node - (i != 0), (float)(i != 0)) * (1.0f / total);
}

// @0x4e2288: the time passed (of the total) becomes how far along the path, by the points' time-to-distance curve and the interpolation
float JumpDefinition::TimeToPgt(float time, float total) {
    if (time > total) time = total;
    float f = 1.0f;
    for (unsigned i = 0; i < m_distanceDistribution.size(); i++) {
        const DistancePoint &p = m_distanceDistribution[i];
        if (time < total * p.timePgt) {
            const DistancePoint &q = m_distanceDistribution[i - 1];
            const float from = total * q.timePgt;
            f = q.distancePgt + ((time - from) / (total * p.timePgt - from)) * (p.distancePgt - q.distancePgt);
            break;
        }
    }
    return jet::core::GetInterpolated((jet::core::Interpolation)m_interpolationType, f);
}

// @0x4e2328
bool JumpDefinition::UpateControl(JumpDefinitionControl &c, float dt) {
    const float total = c.total;
    c.time = math::clamp(dt + c.time, 0.0f, total);
    c.prevPgt = c.pgt;
    c.prevSegment = c.segment;
    c.prevT = c.t;
    c.pgt = TimeToPgt(c.time, total);
    m_path->TravelAlongPath(c.prevSegment, c.prevT, (c.pgt - c.prevPgt) * m_path->GetTotalDistance(), c.segment, c.t, 0);
    return (dt > 0.0f && c.time == c.total) || (dt < 0.0f && c.time == 0.0f);
}

// @0x4e241c
math::vec3<float> JumpDefinition::GetMovementDelta(JumpDefinitionControl &c) {
    return m_path->GetPositionAt(c.prevSegment, c.prevT) - m_path->GetPositionAt(c.segment, c.t);
}

// @0x4e2478
void JumpDefinition::InitCtrl(JumpDefinitionControl &c) { c.total = m_totalTime; }

// @0x4e2484
float JumpDefinition::GetAverageFrontSpeed(JumpDefinitionControl &c) {
    if (c.averageFrontSpeed == 0.0f) {
        math::vec3<float> d = m_path->GetPositionAt(0, 0) - m_path->GetLastInterpolatedPosition();
        d.z = 0;
        c.averageFrontSpeed = d.getLength() / (c.total * 0.001f);
    }
    return c.averageFrontSpeed;
}

// @0x4e2524
void JumpDefinition::InitCtrl(JumpDefinitionControl &c, float frontSpeed) {
    c.total = m_totalTime;
    if (frontSpeed > 0.0f) {
        const float mult = math::clamp(GetAverageFrontSpeed(c) / frontSpeed, m_timeMultMin, m_timeMultMax);
        c.averageFrontSpeed = 0;
        c.total = mult * m_totalTime;
    }
}
