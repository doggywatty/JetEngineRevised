// core/particles/src/particlemgr.cpp
#include "particlemgr.h"
#include "core/util.h"
#include "stream/streammgr.h"
#include "video/driver.h"
#include <algorithm>
#include <boost/bind/bind.hpp>
#include <cmath>
#include <cstring>

namespace ps {

using jet::core::Randf2;

// the defaults of the static initializer @0x3d4a00: the game's world is z up, looking along -y
math::vec3<float> ParticleMgr::s_front(0, -1, 0);
math::vec3<float> ParticleMgr::s_up(0, 0, 1);
math::vec3<float> ParticleMgr::s_right(-1, 0, 0);
math::vec3<float> ParticleMgr::s_gravity(0, 0, -1);
math::vec3<float> ParticleMgr::s_wind(0, 1, 0);

static float FloatBits(unsigned bits) {
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

// @0x684af8: the table of sin / cos of the rotation (1024 half steps), the one of the quantized axis components, and
// the pool of the geometries the quads are written in (50 at most)
ParticleMgr::ParticleMgr()
    : m_computeLocalspacePeriod(1), m_minDistance(0), m_maxDistance(3.4028235e38f), m_particlesToRender(0),
      m_particlesRendered(0), m_systemsToRender(0), m_systemsRendered(0), m_vertexCount(0), m_indexCount(0),
      m_geometryPool(50), m_maxPoolSize(0) {
    m_geometryPool.CreateGeometry = boost::bind(&ParticleMgr::CreateGeometry, this);
    m_sinCos.resize(0x400);
    for (unsigned i = 0; i < m_sinCos.size(); i++) {
        float a = (float)i * FloatBits(0x40c90fdb) / (float)m_sinCos.size();
        m_sinCos[i].sin = sinf(a);
        m_sinCos[i].cos = cosf(a);
    }
    for (int i = 0; i < 256; i++) {
        float a = ((float)i - 127.0f) * FloatBits(0x3c010204);
        float b = ((float)i - 127.0f) * FloatBits(0x3c000000);
        m_byteToFloat[i] = i <= 0x7f ? a : b;
    }
}

// @0x6853d0
ParticleMgr::~ParticleMgr() {}

// @0x6856d4: 16000 quads of 4 vertices (position, colour, two uv sets) in a dynamic buffer and the indices of their two triangles
boost::shared_ptr<jet::video::Geometry> ParticleMgr::CreateGeometry() {
    typedef jet::video::Geometry G;
    boost::shared_ptr<G> g = G::New();
    g->SetVertexCount(0xfa00);
    g->SetIndexCount(0x17700);
    g->AddAttribute(jet::String("positions"), G::SemanticDef::Position, G::UsageDef::Dynamic, G::TypeDef::Float, 3);
    g->AddAttribute(jet::String("colors"), G::SemanticDef::Color, G::UsageDef::Dynamic, G::TypeDef::U8N, 4);
    const G::TypeDef::type uvType = GetUVQuantization() ? G::TypeDef::U16N : G::TypeDef::Float;
    g->AddAttribute(jet::String("uv0"), G::SemanticDef::TexCoord0, G::UsageDef::Dynamic, uvType, 2);
    g->AddAttribute(jet::String("uv1"), G::SemanticDef::TexCoord1, G::UsageDef::Dynamic, uvType, 2);
    g->SetTopology(G::TopologyDef::Triangles);
    g->SetIndexUsage(G::UsageDef::Static);
    g->Compile();
    static const unsigned short k_quad[6] = { 0, 3, 1, 0, 2, 3 };
    unsigned short *idx = g->MapIndicesWrite();
    for (unsigned q = 0, base = 0; base < 0xfa00; q++, base += 4)
        for (int k = 0; k < 6; k++) idx[q * 6 + k] = (unsigned short)(k_quad[k] + base);
    g->UnmapAll();
    return g;
}

// @0x687724: a recycled system, or a clone of the base (loaded from the file the first time)
boost::shared_ptr<ParticleSystem> ParticleMgr::Load(const jet::String &name) {
    TightenArray();
    std::map<jet::String, std::vector<boost::shared_ptr<ParticleSystem> > >::iterator pool = m_reusePool.find(name);
    if (pool != m_reusePool.end() && !pool->second.empty()) {
        boost::shared_ptr<ParticleSystem> sp = pool->second.back();
        pool->second.pop_back();
        m_array.push_back(sp);
        return sp;
    }
    boost::shared_ptr<ParticleSystem> base;
    boost::unordered_map<jet::String, boost::shared_ptr<ParticleSystem> >::iterator it = m_baseMap.find(name);
    if (it != m_baseMap.end()) {
        base = it->second;
    } else {
        jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
        jet::stream::IStream *stream = sm.CreateStream(name);
        if (!stream) stream = sm.CreateStreamByName(name);
        if (!stream) {
            m_baseMap[name] = boost::shared_ptr<ParticleSystem>();
            return boost::shared_ptr<ParticleSystem>();
        }
        base.reset(new ParticleSystem);
        base->SetResourceName(name);
        if (!base->Init(stream) || !base->Load()) return boost::shared_ptr<ParticleSystem>();
        m_baseMap[name] = base;
    }
    if (!base) return base;
    boost::shared_ptr<ParticleSystem> c = base->Clone();
    m_array.push_back(c);
    return c;
}

// @0x6879b8
bool ParticleMgr::CanLoad(const jet::String &name) {
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    return sm.CanCreateStream(name, false) || sm.CanCreateStreamByName(name, false);
}

// @0x687604: count systems are made now and handed to the pool
void ParticleMgr::PreLoad(const jet::String &name, unsigned count) {
    if (!count) return;
    std::vector<boost::shared_ptr<ParticleSystem> > tmp(count);
    for (unsigned i = 0; i < count; i++) tmp[i] = Load(name);
}

// @0x6879f4: the base systems whose files changed are loaded again and the clones follow
void ParticleMgr::ReloadAllChanged() {
    for (boost::unordered_map<jet::String, boost::shared_ptr<ParticleSystem> >::iterator it = m_baseMap.begin(); it != m_baseMap.end(); ++it) {
        boost::shared_ptr<ParticleSystem> &base = it->second;
        if (!base || !base->IsChanged()) continue;
        base->Unload();
        base->Load();
        for (size_t i = 0; i < m_array.size(); i++)
            if (m_array[i]->GetResourceName().Equals(it->first)) m_array[i]->RelinkToBase(*base);
    }
}

// @0x6874e4
void ParticleMgr::ReleasePool() {
    TightenArray();
    m_reusePool.clear();
}

// @0x68751c: the systems nobody but the manager holds are reset and go to the pool of their file
void ParticleMgr::TightenArray() {
    for (int i = (int)m_array.size() - 1; i >= 0; i--) {
        if (i >= (int)m_array.size()) continue;
        boost::shared_ptr<ParticleSystem> sp = m_array[i];
        if (!sp || sp.use_count() != 2) continue;  // the array's and this copy
        sp->Reset();
        std::vector<boost::shared_ptr<ParticleSystem> > &pool = m_reusePool[sp->GetResourceName()];
        if (pool.size() < m_maxPoolSize) pool.push_back(sp);
        for (size_t k = 0; k < m_array.size(); k++)
            if (m_array[k] == sp) { m_array.erase(m_array.begin() + k); break; }
    }
}

// @0x6883dc
void ParticleMgr::SetCoordSystem(const math::vec3<float> &front, const math::vec3<float> &up) {
    s_front = front;
    s_up = up;
    s_right = math::cross(front, up);
}
// @0x6884bc
void ParticleMgr::SetGravityVector(const math::vec3<float> &g) {
    s_gravity = g;
    s_gravity.normalize();
}
// @0x688530
void ParticleMgr::SetWindVector(const math::vec3<float> &w) {
    s_wind = w;
    s_wind.normalize();
}
// @0x68865c
void ParticleMgr::SetUVQuantization(bool v) { ParticleDef::s_uvQuantization = v; }
// @0x68867c
bool ParticleMgr::GetUVQuantization() { return ParticleDef::s_uvQuantization; }

// @0x688614: the period (a power of two) of the recomputation of the axes of the quads
void ParticleMgr::SetComputeLocalspacePeriod(unsigned p) {
    unsigned v = 1;
    if (p == 0) p = 1;
    while (v < p) v <<= 1;
    m_computeLocalspacePeriod = v == 1 ? 1 : v - 1;
}

// @0x6885ac
void ParticleMgr::UnregisterFromRendering(const boost::shared_ptr<ParticleSystem> &ps) {
    std::vector<boost::shared_ptr<ParticleSystem> >::iterator it = std::find(m_toRender.begin(), m_toRender.end(), ps);
    if (it != m_toRender.end()) m_toRender.erase(it);
}
// @0x6885e4
void ParticleMgr::UnregisterFromUpdate(const boost::shared_ptr<ParticleSystem> &ps) {
    std::vector<boost::shared_ptr<ParticleSystem> >::iterator it = std::find(m_toUpdate.begin(), m_toUpdate.end(), ps);
    if (it != m_toUpdate.end()) m_toUpdate.erase(it);
}

// @0x685e24: the registered systems that something else still holds update (the other ones are recycled)
void ParticleMgr::Update(unsigned dtMs) {
    for (size_t i = 0; i < m_toUpdate.size();) {
        boost::shared_ptr<ParticleSystem> ps = m_toUpdate[i];
        const long mine = ps->IsAutoRendering() ? 3 : 2;  // the array, the update list (and the render list); the copy above is one more
        if (ps.use_count() - 1 == mine) {
            ps->Reset();
            ps.reset();
            TightenArray();
            continue;
        }
        if (ps->IsEnabled()) ps->Update(dtMs);
        i++;
    }
}

// @0x685d1c
void ParticleMgr::Render(const CameraData &cd) {
    m_camera = cd;
    BeginRendering();
    for (size_t i = 0; i < m_toRender.size();) {
        boost::shared_ptr<ParticleSystem> ps = m_toRender[i];
        m_systemsToRender++;
        const long mine = ps->IsAutoUpdate() ? 3 : 2;
        if (ps.use_count() - 1 == mine) {
            ps->Reset();
            ps.reset();
            TightenArray();
            continue;
        }
        i++;
        if (!ps->IsEnabled()) continue;
        m_systemsRendered++;
        for (unsigned e = 0; e < ps->GetEmitterCount(); e++)
            if (ps->GetEmitter(e).IsVisible()) AddToRendering(ps->GetEmitter(e));
    }
    EndRendering();
}

// @0x685b64: one system drawn now
void ParticleMgr::Render(const boost::shared_ptr<ParticleSystem> &ps) {
    m_systemsToRender++;
    m_systemsRendered++;
    BeginRendering();
    for (unsigned e = 0; e < ps->GetEmitterCount(); e++)
        if (ps->GetEmitter(e).IsVisible()) AddToRendering(ps->GetEmitter(e));
    EndRendering();
}

// @0x685eb0
void ParticleMgr::BeginRendering() {
    m_particlesToRender = 0;
    m_particlesRendered = 0;
    m_systemsToRender = 0;
    m_systemsRendered = 0;
}

// @0x685a58: the id of a material is its place in the list of the ones seen this frame
int ParticleMgr::GetMaterialId(const jet::video::Material &m) {
    const unsigned hash = m.GetHash();
    for (size_t i = 0; i < m_materialIds.size(); i++)
        if (m_materialIds[i].first == hash && (m_materialIds[i].second == &m || *m_materialIds[i].second == m)) return (int)i;
    m_materialIds.push_back(std::make_pair(hash, &m));
    return (int)m_materialIds.size() - 1;
}

// @0x685ec8: the emitter waits for the end of the frame in the list of its material, with its distance to the camera
void ParticleMgr::AddToRendering(Emitter &e) {
    const int id = GetMaterialId(e.GetParticleDef().GetMaterial());
    std::vector<Emitter *> &list = m_emittersByMaterial[id];
    Shape *shape = e.GetShape();
    shape->UpdateTransform();
    float d = 0;
    if (!m_camera.orthogonal) {
        const math::vec3<float> &w = shape->GetWorldPosition();
        math::vec3<float> v(w.x - m_camera.position.x, w.y - m_camera.position.y, w.z - m_camera.position.z);
        d = sqrtf(v.getLengthSq());
    }
    e.SetCameraDistance(d);
    list.push_back(&e);
    m_particlesToRender += (unsigned)e.GetAllParticles().size();
}

// the farther emitters first
static bool EmitterFartherFirst(const Emitter *a, const Emitter *b) { return a->GetCameraDistance() > b->GetCameraDistance(); }

// @0x68600c: the quads of the emitters of every material are written in one geometry, a job each
void ParticleMgr::EndRendering() {
    if (m_particlesToRender) {
        boost::shared_ptr<jet::video::Geometry> geom = m_geometryPool.GetGeometry();
        if (geom) {
            unsigned n = m_particlesToRender < 16000 ? m_particlesToRender : 16000;
            geom->SetVertexCount(n * 4);
            geom->SetIndexCount(n * 6);
            typedef jet::video::Geometry G;
            const int pos = geom->FindAttributeIdxByName(jet::String("positions"));
            const int col = geom->FindAttributeIdxByName(jet::String("colors"));
            const int uv0 = geom->FindAttributeIdxByName(jet::String("uv0"));
            const int uv1 = geom->FindAttributeIdxByName(jet::String("uv1"));
            m_sink.position = (unsigned char *)geom->MapAttributeFloatWrite(pos); m_sink.positionStride = geom->GetAttributeStride(pos);
            m_sink.color = geom->MapAttributeU8Write(col); m_sink.colorStride = geom->GetAttributeStride(col);
            m_sink.uv0 = (unsigned char *)(GetUVQuantization() ? (void *)geom->MapAttributeU16Write(uv0) : (void *)geom->MapAttributeFloatWrite(uv0));
            m_sink.uv0Stride = geom->GetAttributeStride(uv0);
            m_sink.uv1 = (unsigned char *)(GetUVQuantization() ? (void *)geom->MapAttributeU16Write(uv1) : (void *)geom->MapAttributeFloatWrite(uv1));
            m_sink.uv1Stride = geom->GetAttributeStride(uv1);
            m_jobs.clear();
            m_jobs.resize(100);
            m_vertexCount = 0;
            m_indexCount = 0;
            unsigned jobCount = 0;
            for (std::map<int, std::vector<Emitter *> >::iterator it = m_emittersByMaterial.begin(); it != m_emittersByMaterial.end(); ++it) {
                std::vector<Emitter *> &list = it->second;
                if (list.empty()) continue;
                std::sort(list.begin(), list.end(), EmitterFartherFirst);
                const unsigned start = m_indexCount;
                for (size_t k = 0; k < list.size(); k++) AddToList(*list[k]);
                if (m_indexCount > start && jobCount < m_jobs.size()) {
                    jet::video::RenderJob &job = m_jobs[jobCount++];
                    job.SetGeometry(geom);
                    job.SetDebugModelName(jet::String("ParticleMgr"));
                    job.SetRadius(FloatBits(0x4e6e6b28));
                    ParticleDef &def = list[0]->GetParticleDef();
                    SetMaterialUniforms(def.GetMaterial(), def);
                    job.SetMaterial(&def.GetMaterial());
                    job.SetRange(start, m_indexCount - start);
                }
                list.clear();
            }
            geom->UnmapAll();
            if (jobCount) FlushList(jobCount);
        }
    }
    for (std::map<int, std::vector<Emitter *> >::iterator it = m_emittersByMaterial.begin(); it != m_emittersByMaterial.end(); ++it) it->second.clear();
    m_materialIds.clear();
}

// @0x6863ac
void ParticleMgr::AddToList(Emitter &e) {
    if (e.GetAllParticles().empty()) return;
    AddToList_Worker(e);
}

// @0x687370: the geometry is drawn with the jobs
void ParticleMgr::FlushList(unsigned count) {
    jet::video::Driver *drv = jet::video::Driver::GetCurrent();
    if (!drv || !m_indexCount || !count) return;
    for (unsigned i = 0; i < count; i++) drv->DrawPrimitives(m_jobs[i]);
    m_geometryPool.Reset();
}

// @0x6886ac: the camera position of the shaders that fade by it
void ParticleMgr::SetMaterialUniforms(jet::video::Material &m, const ParticleDef &def) {
    const int idx = def.GetMaterialUniforms();
    if (idx < 0) return;
    m.SetUniform(0, (unsigned)idx, jet::video::ShaderUniform(m_camera.position));
}

// @0x687c90
void ParticleMgr::UnloadGPUResources() {
    m_jobs.clear();
}

// @0x6863cc: the quads of the live particles of an emitter, the nearest ones last. The axes of a quad follow the
// alignment of the definition (the camera, the velocity, a custom front or a custom frame) and are quantized in the
// particle (axes) so that they are only recomputed every few frames.
void ParticleMgr::AddToList_Worker(Emitter &e) {
    const ParticleDef &def = e.GetParticleDef();
    ParticleSystem &system = e.GetParticleSystem();
    const unsigned alignment = def.GetAlignment();
    math::vec3<float> customA(0, 0, 0), customB(0, 0, 0), v24(0, 0, 0), v30(0, 0, 0), v3c(0, 0, 0);
    const math::mat4<float> &psm = system.GetTransform();
    // the rotation of the system (the three columns of its transform)
    const float psr[9] = { psm.m[0], psm.m[1], psm.m[2], psm.m[4], psm.m[5], psm.m[6], psm.m[8], psm.m[9], psm.m[10] };
    auto rotate = [&psr](const math::vec3<float> &v) {
        return math::vec3<float>(psr[0] * v.x + psr[3] * v.y + psr[6] * v.z, psr[1] * v.x + psr[4] * v.y + psr[7] * v.z,
                                 psr[2] * v.x + psr[5] * v.y + psr[8] * v.z);
    };
    if (alignment == ParticleDef::AlignmentDef::CustomFrame) {
        // the system's rotation times the frame's: its columns give the axes
        math::mat3<float> c = def.m_customFrame.getAsMat3();
        float n[9];
        for (int col = 0; col < 3; col++) {
            const float cx = c.m[col * 3], cy = c.m[col * 3 + 1], cz = c.m[col * 3 + 2];
            n[col * 3 + 0] = psr[0] * cx + psr[3] * cy + psr[6] * cz;
            n[col * 3 + 1] = psr[1] * cx + psr[4] * cy + psr[7] * cz;
            n[col * 3 + 2] = psr[2] * cx + psr[5] * cy + psr[8] * cz;
        }
        customA = math::vec3<float>(-n[0], -n[1], -n[2]);
        customB = math::vec3<float>(n[6], n[7], n[8]);
        v24 = math::vec3<float>(-n[3], -n[4], -n[5]);
    } else if (alignment == ParticleDef::AlignmentDef::CustomFront) {
        v24 = def.m_customFront;
    }
    float minDist = def.m_minDistance < m_minDistance ? m_minDistance : def.m_minDistance;
    float maxDist = def.m_maxDistance > m_maxDistance ? m_maxDistance : def.m_maxDistance;
    const bool quantized = GetUVQuantization();
    std::vector<Particle> &particles = e.GetAllParticles();
    const math::vec3<float> &camPos = m_camera.position, &camFront = m_camera.front;
    const unsigned maxQuantized = 0;
    (void)maxQuantized;
    for (int i = (int)particles.size() - 1; i >= 0; i--) {
        Particle &p = particles[i];
        if (p.life >= p.duration) continue;
        if (m_indexCount > 0xffdb) return;
        m_particlesRendered++;
        const unsigned frameIdx = (unsigned)p.life >> 6;
        math::vec3<float> pos = def.GetLocalParticles() ? math::transform(psm, p.position) : p.position;
        const float depth = (pos.x - camPos.x) * camFront.x + (pos.y - camPos.y) * camFront.y + (pos.z - camPos.z) * camFront.z;
        if (depth > maxDist || depth < minDist) continue;
        unsigned axesWord;
        memcpy(&axesWord, p.axes, 4);
        if ((((unsigned)i ^ frameIdx) & m_computeLocalspacePeriod) == 0 || axesWord == 0) {
            const int half = p.rotationFP >> 1;
            const SinCosData &sc = m_sinCos[((unsigned)p.rotationFP >> 17) & 0x3ff];
            const float sn = sc.sin, cs = sc.cos;
            auto quatOf = [&](const math::vec3<float> &axis) { return math::quat<float>(sn * axis.x, sn * axis.y, sn * axis.z, cs); };
            switch (alignment) {
            case ParticleDef::AlignmentDef::Camera: {
                if (m_camera.orthogonal) v24 = math::vec3<float>(-camFront.x, -camFront.y, -camFront.z);
                else v24 = math::normalizedFast(math::vec3<float>(camPos.x - pos.x, camPos.y - pos.y, camPos.z - pos.z));
                v3c = math::cross(v24, m_camera.right);
                if (half) v3c = quatOf(v24) * v3c;
                v30 = math::cross(v24, v3c);
                break;
            }
            case ParticleDef::AlignmentDef::VelocityToCamera: {
                if (m_camera.orthogonal) v24 = math::vec3<float>(-camFront.x, -camFront.y, -camFront.z);
                else v24 = math::normalizedFast(math::vec3<float>(camPos.x - pos.x, camPos.y - pos.y, camPos.z - pos.z));
                v3c = math::normalizedFast(p.velocity);
                if (def.GetLocalParticles()) v3c = rotate(v3c);
                if (half) v24 = quatOf(v3c) * v24;
                v30 = math::normalizedFast(math::cross(v24, v3c));
                break;
            }
            case ParticleDef::AlignmentDef::VelocityAxis: {
                v24 = math::normalizedFast(p.velocity);
                if (def.GetLocalParticles()) v24 = rotate(v24);
                v3c = m_camera.up;
                v30 = math::normalizedFast(math::cross(v24, v3c));
                if (half) v30 = quatOf(v24) * v30;
                v3c = math::cross(v24, v30);
                break;
            }
            case ParticleDef::AlignmentDef::CustomFront: {
                v3c = math::normalizedFast(p.velocity);
                v30 = math::normalizedFast(math::cross(v24, v3c));
                if (half) {
                    math::vec3<float> c = math::cross(v3c, v30);
                    math::quat<float> q = quatOf(c);
                    v3c = q * v3c;
                    v30 = q * v30;
                }
                break;
            }
            default: {  // CustomFrame
                v30 = customA;
                v3c = customB;
                if (half) {
                    math::quat<float> q = quatOf(v24);
                    v3c = q * v3c;
                    v30 = q * v30;
                }
                break;
            }
            }
            p.axes[0] = (unsigned char)((int)(v30.x * 127.0f) + 0x7f);
            p.axes[1] = (unsigned char)((int)(v30.y * 127.0f) + 0x7f);
            p.axes[2] = (unsigned char)((int)(v30.z * 127.0f) + 0x7f);
            p.axes[3] = (unsigned char)((int)(v3c.x * 127.0f) + 0x7f);
            p.axes[4] = (unsigned char)((int)(v3c.y * 127.0f) + 0x7f);
            p.axes[5] = (unsigned char)((int)(v3c.z * 127.0f) + 0x7f);
        }
        const ParticleDefFrame &frame = def.GetFrame(frameIdx);
        u32 seed = p.seed;
        const float r = Randf2(seed);
        const float width = frame.sizeX + r * def.m_sizeVariation.x;
        const float height = frame.sizeY + r * def.m_sizeVariation.y;
        const math::vec3<float> R(width * m_byteToFloat[p.axes[0]], width * m_byteToFloat[p.axes[1]], width * m_byteToFloat[p.axes[2]]);
        const math::vec3<float> U(height * m_byteToFloat[p.axes[3]], height * m_byteToFloat[p.axes[4]], height * m_byteToFloat[p.axes[5]]);
        const math::vec3<float> p0(pos.x - R.x * frame.pivotX - U.x * frame.pivotY, pos.y - R.y * frame.pivotX - U.y * frame.pivotY,
                                   pos.z - R.z * frame.pivotX - U.z * frame.pivotY);
        const math::vec3<float> corner[4] = { p0, p0 + R, p0 + U, (p0 + U) + R };
        // the uv rectangle (a, b, c, d) = (u0, v0, u1, v1): the corners take (c, d) (a, d) (c, b) (a, b)
        unsigned char *vp = m_sink.position + (size_t)m_vertexCount * m_sink.positionStride;
        for (int k = 0; k < 4; k++) {
            const unsigned v = m_vertexCount + (unsigned)k;
            float *pp = (float *)(m_sink.position + (size_t)v * m_sink.positionStride);
            pp[0] = corner[k].x; pp[1] = corner[k].y; pp[2] = corner[k].z;
            memcpy(m_sink.color + (size_t)v * m_sink.colorStride, &frame.color, 4);
            if (quantized) {
                const ParticleDefQuantizedUVFrame &q = def.GetQuantizedUVFrame(frameIdx);
                unsigned short *t0 = (unsigned short *)(m_sink.uv0 + (size_t)v * m_sink.uv0Stride);
                t0[0] = (k & 1) ? q.u0 : q.u1;
                t0[1] = (k & 2) ? q.v0 : q.v1;
                unsigned short *t1 = (unsigned short *)(m_sink.uv1 + (size_t)v * m_sink.uv1Stride);
                t1[0] = frame.extra;
                t1[1] = 0;
            } else {
                const ParticleDefUVFrame &q = def.GetUVFrame(frameIdx);
                float *t0 = (float *)(m_sink.uv0 + (size_t)v * m_sink.uv0Stride);
                t0[0] = (k & 1) ? q.u0 : q.u1;
                t0[1] = (k & 2) ? q.v0 : q.v1;
                float *t1 = (float *)(m_sink.uv1 + (size_t)v * m_sink.uv1Stride);
                t1[0] = (float)frame.extra * FloatBits(0x377ba882);
                t1[1] = 0;
            }
        }
        (void)vp;
        m_vertexCount += 4;
        m_indexCount += 6;
    }
}

}
