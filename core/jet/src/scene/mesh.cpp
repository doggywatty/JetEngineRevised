// core/jet/src/scene/mesh.cpp
#include "scene/mesh.h"
#include "scene/submesh.h"
#include "mem/memorymanager.h"

namespace jet::scene {

Mesh::Mesh(ModelBase *base) : m_base(base), m_isSkinned(false), m_subMeshCount(0), m_subMeshes(0) {}

Mesh::~Mesh() {
    for (unsigned i = 0; i < m_subMeshCount; i++) delete m_subMeshes[i];
    if (m_subMeshes) jet::mem::Free_S(m_subMeshes);
}

// Mesh::Load @0x72cb90
int Mesh::Load(jet::stream::IStream &s) {
    int version = 0;
    s >> version;
    if (version != 100) return 0;
    return LoadV100(s);
}

// Mesh::LoadV100 @0x72cbdc: u8 skinned, u8 (unused), aabb (min, max), u16 submesh count, submeshes
int Mesh::LoadV100(jet::stream::IStream &s) {
    unsigned char skinned = 0, unused = 0;
    s >> skinned;
    m_isSkinned = skinned != 0;
    s >> unused;
    s.Read(&m_boundingBox, 24);
    unsigned short count = 0;
    s >> count;
    m_subMeshCount = count;
    m_subMeshes = (SubMesh**)jet::mem::Malloc_NZ_S(count * sizeof(SubMesh*));
    for (unsigned i = 0; i < count; i++) {
        m_subMeshes[i] = new SubMesh(this);
        m_subMeshes[i]->Load(s);
    }
    return 0;
}

}
