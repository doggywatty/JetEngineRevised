// framework/src/physics/collisionmesh.cpp
#include "physics/collisionmesh.h"
#include "stream/streammgr.h"
#include <cstdio>
#include <algorithm>

// @0x652be8: the volume starts as an empty box of -1..1 (the first vertex resets it)
CollisionMesh::CollisionMesh() : m_numTriangles(0), m_volume(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1)) {}

bool CollisionMesh::Init(jet::stream::IStream *s) {
    SetStream(s);
    return true;
}

// @0x652d7c: the version, then the data of it
bool CollisionMesh::Load() {
    jet::stream::IStream *s = GetStream();
    s->Start();
    s->SeekBeg(0);
    int version = 0;
    *s >> version;
    bool ok = version == 100 && LoadV100(*s);
    s->Finish();
    return ok;
}

// @0x652e08: u32 vertices and their floats; u32 normals (skipped); u8 materials, each a name (u16 length and its bytes) and
// then the ids; u32 triangles, each 4 u32 (3 vertex indices and the normal, which is not kept) and a u8 material; u32 normals (skipped)
bool CollisionMesh::LoadV100(jet::stream::IStream &s) {
    unsigned vertexCount = 0;
    s >> vertexCount;
    m_vertices.resize(vertexCount);
    if (vertexCount) s.Read(&m_vertices[0], vertexCount * 12);
    unsigned skipped = 0;
    s >> skipped;
    s.SeekCur((int)(skipped * 12));
    unsigned char materialCount = 0;
    s.Read1(&materialCount);
    m_materials.assign(materialCount, 0);
    m_materialNames.resize(materialCount);
    for (unsigned i = 0; i < materialCount; i++) {
        unsigned short len = 0;
        s.Read2(&len);
        std::vector<char> name(len + 1, 0);
        s.Read(&name[0], len);
        m_materialNames[i] = jet::String(&name[0]);
    }
    if (materialCount) s.Read(&m_materials[0], materialCount * 4u);
    m_numTriangles = 0;
    s >> m_numTriangles;
    m_indexes.assign(m_numTriangles * 3, 0);
    m_materialIndexes.assign(m_numTriangles, 0);
    for (unsigned t = 0; t < m_numTriangles; t++) {
        unsigned a = 0, b = 0, c = 0, normal = 0;
        s >> a >> b >> c >> normal;
        unsigned char material = 0;
        s.Read1(&material);
        m_indexes[t * 3] = (int)a;
        m_indexes[t * 3 + 1] = (int)b;
        m_indexes[t * 3 + 2] = (int)c;
        m_materialIndexes[t] = material;
    }
    skipped = 0;
    s >> skipped;
    if (skipped) s.SeekCur((int)(skipped * 12));
    m_volume.minPoint = m_volume.maxPoint = m_vertices[0];
    for (size_t i = 1; i < m_vertices.size(); i++) m_volume.addInternalPoint(m_vertices[i]);
    return true;
}

// @0x6531c4: every triangle gets the one material
void CollisionMesh::OverrideMaterial(const jet::String &name, unsigned id) {
    m_materialNames.assign(1, name);
    m_materials.assign(1, id);
    m_materialIndexes.assign(m_materialIndexes.size(), 0);
}

CollisionMeshLoader *CollisionMeshLoader::s_instance = 0;
CollisionMeshLoader *CollisionMeshLoader::GetInstance() {
    if (!s_instance) s_instance = new CollisionMeshLoader;
    return s_instance;
}
void CollisionMeshLoader::FreeInstance() {
    delete s_instance;
    s_instance = 0;
}
CollisionMeshLoader::CollisionMeshLoader() {}
CollisionMeshLoader::~CollisionMeshLoader() { FreeAll(); }

// @0x65358c: a mesh once, however many ask for it; one that does not load is not kept
CollisionMesh *CollisionMeshLoader::Load(const jet::String &name) {
    boost::unordered_map<jet::String, boost::shared_ptr<CollisionMesh> >::iterator it = m_map.find(name);
    CollisionMesh *m = 0;
    if (it == m_map.end()) {
        jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
        if (!s) {
            fprintf(stderr, "ERROR: invalid stream %s\n", name.c_str());
            m_map[name] = boost::shared_ptr<CollisionMesh>();
            return 0;
        }
        m = new CollisionMesh;
        m->SetResourceName(name);
        if (!m->Init(s) || !m->Load()) {
            delete m;
            return 0;
        }
        m_map[name] = boost::shared_ptr<CollisionMesh>(m);
    } else
        m = it->second.get();
    if (m) m_array.push_back(m);
    return m;
}

// @0x653484: one of the users lets go
void CollisionMeshLoader::Free(CollisionMesh *m) {
    if (!m) return;
    std::vector<CollisionMesh*>::iterator it = std::find(m_array.begin(), m_array.end(), m);
    if (it != m_array.end()) m_array.erase(it);
}

// @0x653570
void CollisionMeshLoader::FreeAll() { m_array.clear(); }
