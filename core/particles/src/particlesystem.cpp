// core/particles/src/particlesystem.cpp
#include "particlesystem.h"
#include "particlemgr.h"
#include "application.h"
#include "system.h"
#include "video/driver.h"
#include "video/textureloader.h"
#include <algorithm>
#include <cstring>

namespace ps {

static float FloatBits(unsigned bits) {
    float f;
    memcpy(&f, &bits, 4);
    return f;
}

// @0x688a30
ParticleSystem::ParticleSystem()
    : m_linearVelocity(0, 0, 0), m_isEnabled(true), m_isDone(false), m_isWarmUpDone(false), m_autoRendering(false),
      m_autoUpdate(false), m_rotation(0, 0, 0, 1), m_transformVersion(0), m_cachedTransformVersion(~0u) {}

// @0x688ac8
ParticleSystem::~ParticleSystem() { Clear(); }

// @0x688b5c: the emitters are cloned, the resource stays the base's
boost::shared_ptr<ParticleSystem> ParticleSystem::Clone() const {
    boost::shared_ptr<ParticleSystem> c(new ParticleSystem);
    for (size_t i = 0; i < m_emitters.size(); i++) c->AddEmitter(m_emitters[i]->Clone(c.get()));
    c->SetTransform(GetPosition(), m_rotation);
    c->SetEnabled(m_isEnabled);
    c->SetStream(0);
    c->SetResourceName(const_cast<ParticleSystem*>(this)->GetResourceName());
    c->SetLoaded(const_cast<ParticleSystem*>(this)->IsLoaded());
    return c;
}

// @0x688c24
void ParticleSystem::RelinkToBase(const ParticleSystem &base) {
    for (size_t i = 0; i < m_emitters.size(); i++) delete m_emitters[i];
    m_emitters.clear();
    SetEnabled(base.IsEnabled());
    for (size_t i = 0; i < base.m_emitters.size(); i++) AddEmitter(base.m_emitters[i]->Clone(this));
}

// @0x688c8c
void ParticleSystem::SetLodDistance(float d) {
    for (size_t i = 0; i < m_emitters.size(); i++) m_emitters[i]->SetLodDistance(d);
}

// @0x688cc8: the shapes stop spawning; the particles of the system die at their next update when it is immediate
void ParticleSystem::Stop(bool immediate) {
    for (size_t i = 0; i < m_emitters.size(); i++) {
        m_emitters[i]->Stop();
        if (immediate) m_emitters[i]->DelayedKillAllParticles();
    }
}

// @0x688d14
void ParticleSystem::KillAllParticles() {
    for (size_t i = 0; i < m_emitters.size(); i++) {
        m_emitters[i]->Stop();
        m_emitters[i]->KillAllParticles();
    }
}

// @0x688d54: some shape is not done
bool ParticleSystem::IsEmitting() const {
    for (size_t i = 0; i < m_emitters.size(); i++)
        if (!m_emitters[i]->GetShape()->IsDone()) return true;
    return false;
}

// @0x688d9c
unsigned ParticleSystem::AddEmitter(Emitter *e) {
    e->SetParticleSystem(this);
    m_emitters.push_back(e);
    return (unsigned)m_emitters.size() - 1;
}

// @0x688dd8
void ParticleSystem::RemoveEmitter(unsigned i) {
    if (i >= m_emitters.size()) return;
    delete m_emitters[i];
    m_emitters.erase(m_emitters.begin() + i);
}

// @0x688e28
void ParticleSystem::Clear() {
    KillAllParticles();
    for (size_t i = 0; i < m_emitters.size(); i++) delete m_emitters[i];
    m_emitters.clear();
}

// @0x688e40: the rotation part of the transform (the translation is the position)
void ParticleSystem::_UpdateTransform() const {
    math::mat3<float> r = m_rotation.getAsMat3();
    for (int c = 0; c < 3; c++)
        for (int k = 0; k < 3; k++) m_transform.m[c * 4 + k] = r.m[c * 3 + k];
}

// @0x68a458
const math::mat4<float> &ParticleSystem::GetTransform() const {
    if (m_transformVersion != m_cachedTransformVersion) {
        m_cachedTransformVersion = m_transformVersion;
        _UpdateTransform();
    }
    return m_transform;
}

// @0x688ea8
void ParticleSystem::SetPosition(const math::vec3<float> &p) {
    const math::vec3<float> &cur = GetPosition();
    if (cur.x == p.x && cur.y == p.y && cur.z == p.z) return;
    m_transform.setTranslation(p);
    m_transformVersion++;
}

// @0x688f10
void ParticleSystem::SetRotation(const math::quat<float> &q) {
    if (!memcmp(&m_rotation, &q, sizeof(q))) return;
    m_rotation = q;
    m_transformVersion++;
}

// @0x688f4c
void ParticleSystem::SetTransform(const math::vec3<float> &p, const math::quat<float> &q) {
    const math::vec3<float> &cur = GetPosition();
    if (!memcmp(&m_rotation, &q, sizeof(q)) && cur.x == p.x && cur.y == p.y && cur.z == p.z) return;
    m_transform.setTranslation(p);
    m_rotation = q;
    m_transformVersion++;
}

// @0x688fe0
void ParticleSystem::Update() { Update(jet::System::GetApplication()->GetUpdateTime()); }

// @0x689020: the emitters update (at most 50 ms at a time); the system is done when no shape spawns and no particle lives.
// The first update warms the emitters up
void ParticleSystem::Update(unsigned ms) {
    if (m_isDone || !m_isEnabled) return;
    if (!m_isWarmUpDone) {
        for (size_t i = 0; i < m_emitters.size(); i++) {
            unsigned left = m_emitters[i]->GetShape()->GetWarmUpDuration();
            while (left) {
                unsigned step = left < 0x21 ? left : 0x21;
                left -= step;
                m_emitters[i]->Update((float)(int)step * FloatBits(0x3a83126f), step);
            }
        }
        m_isWarmUpDone = true;
    }
    unsigned dt = ms > 50 ? 50 : ms;
    if (!dt) return;
    const float seconds = (float)dt * FloatBits(0x3a83126f);
    bool allDone = true;
    for (size_t i = 0; i < m_emitters.size(); i++) {
        Emitter *e = m_emitters[i];
        if (e->GetShape()->IsDone() && e->GetAllParticles().empty()) continue;
        if (!e->IsVisible()) continue;
        e->Update(seconds, dt);
        if (!e->GetShape()->IsDone() || !e->GetAllParticles().empty()) allDone = false;
    }
    SetDone(allDone);
}

// @0x6891c0: a system the manager does not draw by itself is drawn now
void ParticleSystem::Render() {
    if (m_autoRendering || !m_isEnabled) return;
    boost::shared_ptr<ParticleSystem> self = weak_from_this().lock();
    if (self) ParticleMgr::GetInstance()->Render(self);
}

// @0x689230
void ParticleSystem::Reset() {
    SetDone(false);
    SetEnabled(true);
    SetAutoRendering(false);
    SetAutoUpdate(false);
    SetWarmUpDone(false);
    for (size_t i = 0; i < m_emitters.size(); i++) m_emitters[i]->Reset();
}

// @0x68a2cc
void ParticleSystem::SetAutoRendering(bool v) {
    if (m_autoRendering == v) return;
    ParticleMgr *mgr = ParticleMgr::GetInstance();
    if (!mgr) return;
    m_autoRendering = v;
    boost::shared_ptr<ParticleSystem> self = weak_from_this().lock();
    if (!self) return;
    if (v) mgr->RegisterForRendering(self);
    else mgr->UnregisterFromRendering(self);
}

// @0x68a374
void ParticleSystem::SetAutoUpdate(bool v) {
    if (m_autoUpdate == v) return;
    ParticleMgr *mgr = ParticleMgr::GetInstance();
    if (!mgr) return;
    m_autoUpdate = v;
    boost::shared_ptr<ParticleSystem> self = weak_from_this().lock();
    if (!self) return;
    if (v) mgr->RegisterForUpdate(self);
    else mgr->UnregisterFromUpdate(self);
}

// @0x6892cc: the version, then the emitters
bool ParticleSystem::Load() {
    jet::stream::IStream *s = GetStream();
    s->Start();
    s->SeekBeg(0);
    unsigned version = 0;
    *s >> version;
    bool ok = version == 100 ? LoadV100(*s) : false;
    s->Finish();
    return ok;
}

// @0x689370: every emitter is added (to look at its system) before it loads; the system starts disabled
bool ParticleSystem::LoadV100(jet::stream::IStream &s) {
    unsigned short count = 0;
    s >> count;
    for (unsigned i = 0; i < count; i++) {
        Emitter *e = new Emitter;
        e->SetLodDistance(20.0f);
        AddEmitter(e);
        LoadEmitter(*e, s);
    }
    SetEnabled(false);
    return true;
}

// @0x689410: the definition, the shape (a letter: d disk, s sphere, b box, r rectangle) and the influences
// (m magnet, f force, b brownian, g gravity, w wind)
void ParticleSystem::LoadEmitter(Emitter &e, jet::stream::IStream &s) {
    boost::shared_ptr<ParticleDef> def(new ParticleDef);
    e.SetParticleDef(def);
    LoadParticleDef(*def, s);
    e.SetLodDistance(def->GetLodDistance());
    unsigned char tag = 0;
    s >> tag;
    if (tag == 'd') {
        Disk *sh = new Disk;
        e.SetShape(sh);
        LoadShape(*sh, s);
        float radius = 0, angle = 0;
        s >> radius >> angle;
        sh->SetRadius(radius);
        sh->SetAngle(angle);
        math::quat<float> q(0, 0, 0, 1);
        s >> q;
        sh->SetRotation(q);
    } else if (tag == 's') {
        Sphere *sh = new Sphere;
        e.SetShape(sh);
        LoadShape(*sh, s);
        float radius = 0, angle = 0;
        s >> radius >> angle;
        sh->SetRadius(radius);
        sh->SetAngle(angle);
        math::quat<float> q(0, 0, 0, 1);
        s >> q;
        sh->SetRotation(q);
    } else if (tag == 'b') {
        Box *sh = new Box;
        e.SetShape(sh);
        LoadShape(*sh, s);
        math::vec3<float> size(0, 0, 0);
        s >> size;
        sh->SetSize(size);
        math::quat<float> q(0, 0, 0, 1);
        s >> q;
        sh->SetRotation(q);
    } else if (tag == 'r') {
        Rectangle *sh = new Rectangle;
        e.SetShape(sh);
        LoadShape(*sh, s);
        math::vec2<float> size(0, 0);
        s >> size;
        sh->SetSize(size);
        math::quat<float> q(0, 0, 0, 1);
        s >> q;
        sh->SetRotation(q);
    }
    unsigned influences = 0;
    s >> influences;
    for (unsigned i = 0; i < influences; i++) {
        unsigned char t = 0;
        s >> t;
        if (t == 'm') {
            Magnet *m = new Magnet;
            e.AddInfluence(m);
            math::vec3<float> pos(0, 0, 0);
            float force = 0, maxForce = 0;
            s >> pos >> force >> maxForce;
            m->SetForce(force);
            m->SetMaxForce(maxForce);
            m->SetPosition(pos);
        } else if (t == 'f') {
            Force *f = new Force;
            e.AddInfluence(f);
            math::quat<float> q(0, 0, 0, 1);
            float force = 0;
            s >> q >> force;
            f->SetForce(force);
            f->SetRotation(q);
        } else if (t == 'b') {
            Brownian *b = new Brownian;
            e.AddInfluence(b);
            float fmin = 0, fmax = 0;
            unsigned tmin = 0, tmax = 0;
            s >> fmin >> fmax >> tmin >> tmax;
            b->SetForceInterval(fmin, fmax);
            b->SetTimeInterval(tmin, tmax);
        } else if (t == 'g') {
            Gravity *g = new Gravity;
            e.AddInfluence(g);
            float force = 0;
            s >> force;
            g->SetForce(force);
        } else if (t == 'w') {
            Wind *w = new Wind;
            e.AddInfluence(w);
            float force = 0;
            s >> force;
            w->SetForce(force);
        }
    }
}

// @0x689e8c: what every shape starts with
void ParticleSystem::LoadShape(Shape &sh, jet::stream::IStream &s) {
    unsigned duration = 0, warmUp = 0, emitStart = 0, emitEnd = 0, count = 0, source = 0;
    bool looped = false;
    float speed = 0;
    math::vec3<float> pos(0, 0, 0);
    s >> duration >> warmUp >> emitStart >> emitEnd;
    s >> looped;
    s >> pos;
    s >> count;
    s >> speed;
    s >> source;
    sh.SetDuration(duration);
    sh.SetLife(0);
    sh.SetWarmUpDuration(warmUp);
    sh.SetEmitInterval(emitStart, emitEnd);
    sh.SetLooped(looped);
    sh.SetPosition(pos);
    sh.SetParticleCount(count);
    sh.SetSpeed(speed);
    sh.SetEmitSource(source);
}

// @0x68972c
void ParticleSystem::LoadParticleDef(ParticleDef &d, jet::stream::IStream &s) {
    const float k = FloatBits(0x4322fae1);  // rotation steps per radian
    bool b = false;
    s >> b;
    d.m_localParticles = b;
    float rotA = 0, rotB = 0;
    s >> rotA >> rotB;
    float lo = rotA < rotB ? rotA : rotB, hi = rotA < rotB ? rotB : rotA;
    d.m_initialRotationMin = (int)(lo * k) << 16;
    d.m_initialRotationMax = (int)(hi * k) << 16;
    s >> b;
    d.m_randomRotationDirection = b;
    unsigned u = 0;
    s >> u;
    d.m_alignment = u;
    s >> d.m_alignmentSpeed;
    s >> d.m_linearMomentumFactor;
    s >> d.m_customFront;
    math::quat<float> q(0, 0, 0, 1);
    s >> q;
    d.m_customFrame = q;
    s >> d.m_lodDistance;
    float minDist = 0, maxDist = 0, fade = 0;
    s >> minDist >> maxDist;
    float max = minDist + 1.0f;
    if (max < maxDist) max = maxDist;
    d.m_minDistance = minDist;
    d.m_maxDistance = max;
    s >> fade;
    float half = (max - minDist) * 0.5f;
    if (half > fade) half = fade;
    d.m_fadeRange = half;
    s >> d.m_sizeVariation;
    s >> d.m_rotationSpeedVariation;
    d.m_hasRotationSpeedVariation = (d.m_rotationSpeedVariation < 0 ? -d.m_rotationSpeedVariation : d.m_rotationSpeedVariation) > FloatBits(0x3727c5ac);
    unsigned frames = 0;
    s >> frames;
    d.SetFrameCount(frames);
    for (unsigned i = 0; i < frames; i++) {
        unsigned color = 0;
        float drag = 0, speedScale = 0, rotSpeed = 0;
        math::vec2<float> size(0, 0), pivot(0, 0);
        unsigned char raw[8] = { 0 };
        unsigned short extra = 0;
        s >> color >> drag >> speedScale >> size >> pivot >> rotSpeed;
        s.Read(raw, 4);
        s.Read(raw + 4, 4);
        s >> extra;
        ParticleDefFrame &f = d.GetFrame(i);
        f.color = (color & 0xff00ff00) | ((color & 0xff) << 16) | ((color >> 16) & 0xff);
        f.drag = drag;
        f.speedScale = speedScale;
        f.sizeX = size.x;
        f.sizeY = size.y;
        f.quarterSize = (size.x + size.y) * 0.25f;
        f.pivotX = pivot.x;
        f.pivotY = pivot.y;
        f.rotationSpeed = (int)(rotSpeed * k) << 16;
        f.extra = extra;
        unsigned short uv[4];
        memcpy(uv, raw, 8);
        if (ParticleDef::s_uvQuantization) {
            ParticleDefQuantizedUVFrame &q8 = d.GetQuantizedUVFrame(i);
            q8.u0 = uv[0]; q8.v0 = uv[1]; q8.u1 = uv[2]; q8.v1 = uv[3];
        } else {
            const float scale = FloatBits(0x37800080);
            ParticleDefUVFrame &f16 = d.GetUVFrame(i);
            f16.u0 = uv[0] * scale; f16.v0 = uv[1] * scale; f16.u1 = uv[2] * scale; f16.v1 = uv[3] * scale;
        }
    }
    unsigned blending = 0;
    s >> blending;
    jet::String technique;
    s >> technique;
    d.SetRenderTechnique(technique, blending);
    jet::video::Material &mat = d.GetMaterial();
    unsigned short count = 0;
    s >> count;
    for (unsigned i = 0; i < count; i++) {
        jet::String name;
        float value = 0;
        s >> name >> value;
        int idx = mat.FindUniformIdxByName(0, name);
        if (idx >= 0) mat.SetUniform(0, (unsigned)idx, jet::video::ShaderUniform(value));
    }
    s >> count;
    jet::video::TextureLoader *loader = jet::video::TextureLoader::GetInstance();
    for (unsigned i = 0; i < count; i++) {
        jet::String sampler, tex;
        s >> sampler >> tex;
        if (tex.empty() || tex.Equals(jet::String("unused")) || tex.Equals(jet::String("- None -"))) continue;
        boost::shared_ptr<jet::video::Texture> t = loader->Load(tex);
        if (i >= mat.GetTextureLayerCount(0)) continue;
        jet::video::TextureLayer &layer = mat.GetTextureLayer(0, i);
        layer.SetTexture(t);
        jet::video::Driver *drv = jet::video::Driver::GetCurrent();
        if (drv && drv->GetSeparateAlphaTextures()) {
            jet::String alpha(tex);
            alpha.append("__alpha");
            if (loader->CanLoad(alpha)) layer.SetAlphaTexture(loader->Load(alpha));
        }
    }
    bool noCulling = false;
    s >> noCulling;
    if (mat.IsValid()) mat.GetRenderState(0).SetCulling(!noCulling);
}

}
