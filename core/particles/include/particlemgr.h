// core/particles/include/particlemgr.h
#ifndef CORE_PARTICLES_INCLUDE_PARTICLEMGR_H
#define CORE_PARTICLES_INCLUDE_PARTICLEMGR_H

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "jsingleton.h"
#include "smath.h"
#include "core/pstring.h"
#include "particlesystem.h"
#include "video/geometrypool.h"
#include "video/material.h"
#include "video/renderjob.h"

namespace ps {

// What the manager needs of the camera a system is seen from (0x50 bytes; GameLevel::RenderParticles and
// MenuMgr::SetCameraDataForParticles fill it)
struct CameraData {
    math::vec3<float> position;      // 0x0
    math::vec3<float> front;         // 0xc
    math::vec3<float> up;            // 0x18
    math::vec3<float> right;         // 0x24
    math::quat<float> rotation;      // 0x30
    int width, height;               // 0x40 0x44 the viewport
    float fov;                       // 0x48
    bool orthogonal;                 // 0x4c
    CameraData() : position(0, 0, 0), front(0, 0, 0), up(0, 0, 0), right(0, 0, 0), rotation(0, 0, 0, 1), width(0), height(0), fov(0), orthogonal(false) {}
};

// Loads the particle systems (one base per file, clones for the users, recycled when nobody holds them any more),
// updates the ones registered for it and draws every visible emitter in batches by material.
class ParticleMgr : public Singleton<ParticleMgr> {
public:
    struct SinCosData { float sin, cos; };

    ParticleMgr();                                     // @0x684af8
    virtual ~ParticleMgr();                            // @0x6853d0
    void Init() {}                                     // @0x685b60
    void ReloadAllChanged();                           // @0x6879f4
    void PreLoad(const jet::String &name, unsigned count);   // @0x687604
    boost::shared_ptr<ParticleSystem> Load(const jet::String &name);   // @0x687724
    bool CanLoad(const jet::String &name);             // @0x6879b8
    void ReleasePool();                                // @0x6874e4
    void SetMaxPoolSize(unsigned n) { m_maxPoolSize = n; }    // @0x6874dc

    static void SetCoordSystem(const math::vec3<float> &front, const math::vec3<float> &up);   // @0x6883dc
    static void SetGravityVector(const math::vec3<float> &g);   // @0x6884bc
    static void SetWindVector(const math::vec3<float> &w);      // @0x688530
    static const math::vec3<float> &GetFrontVector() { return s_front; }    // @0x688468
    static const math::vec3<float> &GetUpVector() { return s_up; }          // @0x6884a0
    static const math::vec3<float> &GetRightVector() { return s_right; }   // @0x688484
    static const math::vec3<float> &GetGravityVector() { return s_gravity; }   // @0x688514
    static const math::vec3<float> &GetWindVector() { return s_wind; }         // @0x688588
    static void SetUVQuantization(bool v);             // @0x68865c
    static bool GetUVQuantization();                   // @0x68867c
    void SetComputeLocalspacePeriod(unsigned p);       // @0x688614
    void SetMinDistance(float d) { m_minDistance = d; }    // @0x688644
    void SetMaxDistance(float d) { m_maxDistance = d; }    // @0x688650

    void SetCameraData(const CameraData &c) { m_camera = c; }   // @0x685d10
    void Update(unsigned dtMs);                        // @0x685e24
    void Render(const CameraData &c);                  // @0x685d1c
    void Render(const boost::shared_ptr<ParticleSystem> &ps);   // @0x685b64
    void RegisterForRendering(const boost::shared_ptr<ParticleSystem> &ps) { m_toRender.push_back(ps); }   // @0x6885a4
    void UnregisterFromRendering(const boost::shared_ptr<ParticleSystem> &ps);   // @0x6885ac
    void RegisterForUpdate(const boost::shared_ptr<ParticleSystem> &ps) { m_toUpdate.push_back(ps); }      // @0x6885d8
    void UnregisterFromUpdate(const boost::shared_ptr<ParticleSystem> &ps);      // @0x6885e4
    void UnloadGPUResources();                         // @0x687c90

private:
    void TightenArray();                               // @0x68751c
    void BeginRendering();                             // @0x685eb0
    void AddToRendering(Emitter &e);                   // @0x685ec8
    void EndRendering();                               // @0x68600c
    void AddToList(Emitter &e);                        // @0x6863ac
    void AddToList_Worker(Emitter &e);                 // @0x6863cc
    void FlushList(unsigned count);                    // @0x687370
    void SetMaterialUniforms(jet::video::Material &m, const ParticleDef &def);   // @0x6886ac
    int GetMaterialId(const jet::video::Material &m);  // @0x685a58
    boost::shared_ptr<jet::video::Geometry> CreateGeometry();   // @0x6856d4

    // the destination of a quad being written: where every attribute of the geometry starts and its stride
    struct VertexSink {
        unsigned char *position; unsigned positionStride;
        unsigned char *color; unsigned colorStride;
        unsigned char *uv0; unsigned uv0Stride;
        unsigned char *uv1; unsigned uv1Stride;
    };

    static math::vec3<float> s_front, s_up, s_right, s_gravity, s_wind;

    std::vector<SinCosData> m_sinCos;                  // 0x4: 1024 entries
    float m_byteToFloat[256];                          // 0x10: a quantized axis component
    unsigned m_computeLocalspacePeriod;                // 0x414
    float m_minDistance, m_maxDistance;                // 0x418 0x41c
    std::vector<boost::shared_ptr<ParticleSystem> > m_toRender;     // 0x420
    std::vector<boost::shared_ptr<ParticleSystem> > m_toUpdate;     // 0x42c
    unsigned m_particlesToRender, m_particlesRendered, m_systemsToRender, m_systemsRendered;   // 0x444..0x450
    std::map<int, std::vector<Emitter *> > m_emittersByMaterial;    // 0x454
    unsigned m_vertexCount, m_indexCount;              // 0x46c 0x470
    jet::video::GeometryPool m_geometryPool;           // 0x474
    std::vector<jet::video::RenderJob> m_jobs;         // 0x498
    boost::unordered_map<jet::String, boost::shared_ptr<ParticleSystem> > m_baseMap;   // 0x4a4
    std::vector<boost::shared_ptr<ParticleSystem> > m_array;        // 0x4bc
    std::map<jet::String, std::vector<boost::shared_ptr<ParticleSystem> > > m_reusePool;   // 0x4c8
    std::vector<std::pair<unsigned, const jet::video::Material *> > m_materialIds;   // 0x4e0
    CameraData m_camera;                               // 0x5dc
    unsigned m_maxPoolSize;                            // 0x62c
    VertexSink m_sink;
};
}

#endif
