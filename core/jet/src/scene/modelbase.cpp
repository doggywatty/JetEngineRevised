// core/jet/src/scene/modelbase.cpp
#include "scene/modelbase.h"
#include "scene/mesh.h"
#include "scene/submesh.h"
#include "scene/model.h"
#include "stream/istream.h"
#include "video/textureloader.h"
#include "video/driver.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>

namespace jet::scene {

// Technique of the materials a model makes for itself (static string at 0xd13d84)
static const char *k_defaultTechnique = "_default3d";

ModelBase::ModelBase()
    : m_maxLod(1), m_triangleCount(0), m_dips(0), m_geometryCPUAccess(false), m_loadRenderTechniques(true), m_loadTextures(true) {}

ModelBase::~ModelBase() { Unload(); }

bool ModelBase::Init(jet::stream::IStream *s) {
    SetStream(s);
    return true;
}

// Load @0x7018e0
bool ModelBase::Load() {
    jet::stream::IStream *s = GetStream();
    if (!s) return false;
    s->Start();
    s->SeekBeg(0);
    int version = 0;
    *s >> version;
    bool ok = version == 100 && LoadV100(*s);
    if (ok) SetLoaded(true);
    // the stream stays open: submesh geometry is read from it on demand
    return ok;
}

void ModelBase::Unload() {
    m_multiResMeshes.clear();
    m_nodes.clear();
    m_dummies.clear();
    m_cameras.clear();
    m_userData.clear();
    m_nameNodeIdxMap.clear();
    m_nameCameraIdxMap.clear();
    ReleaseMaterials();
    m_triangleCount = m_dips = 0;
    if (GetStream() && GetStream()->IsStarted()) GetStream()->Finish();
    SetLoaded(false);
}

// cmp_dummies @0x701a40
static bool cmp_dummies(const ModelBase::DummyData &a, const ModelBase::DummyData &b) {
    if (!a.name.Equals(b.name)) return false;
    return (unsigned)a.number < (unsigned)b.number;
}

// LoadV100 @0x701a70
bool ModelBase::LoadV100(jet::stream::IStream &s) {
    unsigned short nodeCount = 0;
    s >> nodeCount;
    m_nodes.resize(nodeCount);
    unsigned dummyCount = 0;
    for (unsigned i = 0; i < nodeCount; i++) {
        LoadNodeV100(m_nodes[i], s);
        m_nameNodeIdxMap[m_nodes[i].name] = i;
        if (m_nodes[i].parentIdx >= 0) m_nodes[m_nodes[i].parentIdx].children.push_back(i);
        if (m_nodes[i].flags & NodeFlags::Dummy) ++dummyCount;
    }

    // Dummies by name, a trailing number split off ("Link_03" -> "Link_", 3)
    m_dummies.resize(dummyCount);
    unsigned d = 0;
    for (unsigned i = 0; i < nodeCount; i++) {
        if (!(m_nodes[i].flags & NodeFlags::Dummy)) continue;
        DummyData &dd = m_dummies[d++];
        dd.nodeIdx = i;
        const jet::String &name = m_nodes[i].name;
        unsigned n = name.size();
        if (!n) continue;
        const char *c = name.c_str();
        int j = (int)n - 1;
        while (j >= 0 && isdigit((unsigned char)c[j])) --j;
        if (j == (int)n - 1) {
            dd.number = 0;
            dd.name = name;
        } else {
            dd.number = atoi(c + j + 1);
            dd.name = jet::String(c, c + j + 1);
        }
    }
    std::sort(m_dummies.begin(), m_dummies.end(), cmp_dummies);

    unsigned char maxLod = 1;
    s >> maxLod;
    m_maxLod = maxLod;

    unsigned short meshCount = 0;
    s >> meshCount;
    m_multiResMeshes.resize(meshCount);
    for (unsigned i = 0; i < meshCount; i++) LoadMultiResMeshV100(m_multiResMeshes[i], s);

    unsigned short cameraCount = 0;
    s >> cameraCount;
    m_cameras.resize(cameraCount);
    for (unsigned i = 0; i < cameraCount; i++) {
        LoadCameraV100(m_cameras[i], s);
        m_nameCameraIdxMap[m_cameras[i].name] = i;
    }

    if (s.Tell() < s.Size()) {
        unsigned short count = 0;
        s >> count;
        m_userData.resize(count);
        for (unsigned i = 0; i < count; i++) s >> m_userData[i].key >> m_userData[i].value;
    }
    return true;
}

// LoadNodeV100 @0x701fd0
bool ModelBase::LoadNodeV100(NodeData &n, jet::stream::IStream &s) {
    int version = 0;
    s >> version;
    if (version != 100) return false;
    s >> n.name;
    signed char flags = 0;
    s.Read1(&flags);
    n.flags = (int)flags;
    if (n.name.Equals("camera")) n.flags |= NodeFlags::Camera;
    short parent = -1;
    s.Read2(&parent);
    n.parentIdx = parent;
    s >> n.position;
    s.Read(&n.rotation, 16);
    s >> n.scale;
    s >> n.radius;
    unsigned short props = 0;
    s >> props;
    for (unsigned i = 0; i < props; i++) {  // per-node properties are skipped
        jet::String k, v;
        s >> k >> v;
    }
    return true;
}

// LoadCameraV100 @0x7020f4: the file stores the horizontal fov in degrees
bool ModelBase::LoadCameraV100(CameraData &c, jet::stream::IStream &s) {
    if (!LoadNodeV100(c, s)) return false;
    float hfov = 0;
    s >> c.targetName;
    s >> hfov;
    s >> c.aspectRatio;
    s >> c.nearDist;
    s >> c.farDist;
    unsigned char ortho = 0;
    s >> ortho;
    c.orthogonal = ortho != 0;
    c.fov = 2.0f * atanf(tanf(hfov * 0.0087266471f) / c.aspectRatio);
    return true;
}

// LoadMultiResMeshV100 @0x7021ac
bool ModelBase::LoadMultiResMeshV100(MultiResMeshData &m, jet::stream::IStream &s) {
    int version = 0;
    s >> version;
    if (version != 100) return false;
    m.lods.resize(k_maxLodCount);
    unsigned nodeIdx = 0;
    s >> nodeIdx;
    m.nodeIdx = (unsigned short)nodeIdx;
    unsigned short count = 0;
    s >> count;
    for (unsigned i = 0; i < count; i++) {
        unsigned char lod = 0;
        s.Read1(&lod);
        unsigned slot = (lod >= 1 && lod <= 5) ? lod : 0;  // table at 0xc969b8: 1..5
        boost::shared_ptr<Mesh> mesh(new Mesh(this));
        mesh->Load(s);
        for (unsigned j = 0; j < mesh->GetSubMeshCount(); j++) m_triangleCount += mesh->GetSubMesh(j).GetIndexCount() / 3;
        m_dips += mesh->GetSubMeshCount();
        CollectMaterials(*mesh);
        m.lods[slot] = mesh;
    }
    return true;
}

// AddMaterial @0x701794: a material of the default technique, named after the submesh material
ModelBase::MaterialData *ModelBase::AddMaterial(const jet::String &name) {
    MaterialData md;
    md.name = name;
    if (m_loadRenderTechniques) md.material.SetRenderTechnique(jet::String(k_defaultTechnique));
    m_materials.push_back(md);
    m_nameMaterialIdxMap[name] = (unsigned)m_materials.size() - 1;
    return &m_materials.back();
}

// CollectMaterials @0x7023c0: one material per distinct submesh material name, with the texture
// names of the first submesh using it; when textures are loaded, name i goes to pass 0's layer i
// (and its "__alpha" file to the layer's alpha texture when the driver separates alpha).
void ModelBase::CollectMaterials(const Mesh &mesh) {
    for (unsigned i = 0; i < mesh.GetSubMeshCount(); i++) {
        const SubMesh &sm = mesh.GetSubMesh(i);
        if (m_nameMaterialIdxMap.find(sm.GetMaterialName()) != m_nameMaterialIdxMap.end()) continue;
        MaterialData *md = AddMaterial(sm.GetMaterialName());
        md->textureNames.reserve(sm.GetTextureCount());
        for (unsigned t = 0; t < sm.GetTextureCount(); t++) {
            const jet::String &name = sm.GetTextureName(t);
            md->textureNames.push_back(name);
            if (!m_loadTextures || !md->material.GetRenderTechnique() || !md->material.GetRenderPassCount()) continue;
            if (t >= md->material.GetTextureLayerCount(0)) continue;
            jet::video::TextureLayer &layer = md->material.GetTextureLayer(0, t);
            boost::shared_ptr<jet::video::Texture> tex = jet::video::TextureLoader::GetInstance()->Load(name);
            if (tex) layer.SetTexture(tex);
            if (jet::video::Driver::GetCurrent()->GetSeparateAlphaTextures()) {
                jet::String alpha = name + "__alpha";
                if (jet::video::TextureLoader::GetInstance()->CanLoad(alpha))
                    layer.SetAlphaTexture(jet::video::TextureLoader::GetInstance()->Load(alpha));
            }
        }
    }
}

void ModelBase::ReleaseMaterials() {
    m_materials.clear();
    m_nameMaterialIdxMap.clear();
}

int ModelBase::FindNodeIdxByName(const jet::String &n) const {
    boost::unordered_map<jet::String, unsigned>::const_iterator it = m_nameNodeIdxMap.find(n);
    return it == m_nameNodeIdxMap.end() ? -1 : (int)it->second;
}
int ModelBase::FindCameraIdxByName(const jet::String &n) const {
    boost::unordered_map<jet::String, unsigned>::const_iterator it = m_nameCameraIdxMap.find(n);
    return it == m_nameCameraIdxMap.end() ? -1 : (int)it->second;
}
int ModelBase::FindMaterialIdxByName(const jet::String &n) const {
    boost::unordered_map<jet::String, unsigned>::const_iterator it = m_nameMaterialIdxMap.find(n);
    return it == m_nameMaterialIdxMap.end() ? -1 : (int)it->second;
}
const ModelBase::UserData *ModelBase::FindUserDataByKey(const jet::String &k) const {
    for (unsigned i = 0; i < m_userData.size(); i++)
        if (m_userData[i].key.Equals(k)) return &m_userData[i];
    return 0;
}

// CreateInstance @0x701534
Model *ModelBase::CreateInstance() {
    Model *m = new Model(boost::static_pointer_cast<const ModelBase>(shared_from_this()));
    m_instances.push_back(m);
    return m;
}

void ModelBase::RemoveInstance(Model *m) const {
    std::vector<Model*>::iterator it = std::find(m_instances.begin(), m_instances.end(), m);
    if (it != m_instances.end()) m_instances.erase(it);
}

}
