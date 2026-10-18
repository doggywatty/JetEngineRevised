// core/jet/include/scene/modelbase.h
#ifndef CORE_JET_INCLUDE_SCENE_MODELBASE_H
#define CORE_JET_INCLUDE_SCENE_MODELBASE_H

#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "smath.h"
#include "jenum.h"
#include "core/pstring.h"
#include "res/resource.h"
#include "video/material.h"

namespace jet::scene {
class Model;
class Mesh;

// The shared, loaded part of a .pig model (ModelBase in the 0.0.3 .so, LoadV100 @0x701a70):
//   int version (100)
//   u16 node count, nodes (LoadNodeV100)
//   u8 max lod
//   u16 multires mesh count, multires meshes (LoadMultiResMeshV100)
//   u16 camera count, cameras (LoadCameraV100)
//   [u16 user data count, (key, value) string pairs]   -- when the stream has more data
// Nodes flagged as dummies (flag bit 0) are also listed by name with any trailing number split off.
class ModelBase : public jet::res::Resource, public boost::enable_shared_from_this<ModelBase> {
public:
    struct NodeFlags { enum { Dummy = 1, Skeletal = 4, Camera = 0x80 }; };
    struct NodeData {  // 0x44
        jet::String name;
        unsigned flags;
        int parentIdx;
        math::vec3<float> position;
        math::quat<float> rotation;
        math::vec3<float> scale;
        float radius;
        std::vector<unsigned> children;
        NodeData() : flags(0), parentIdx(-1), position(0, 0, 0), scale(1, 1, 1), radius(0) {}
    };
    struct CameraData : NodeData {  // 0x5c
        jet::String targetName;
        float fov;          // vertical, radians
        float aspectRatio;
        float nearDist;
        float farDist;
        bool orthogonal;
        CameraData() : fov(0), aspectRatio(1), nearDist(0), farDist(0), orthogonal(false) {}
    };
    struct DummyData {  // 0xc: name without trailing digits, the digits, node index
        jet::String name;
        int number;
        unsigned nodeIdx;
    };
    struct MultiResMeshData {  // 0x10: one Mesh per lod slot, and the node the meshes hang on
        std::vector<boost::shared_ptr<Mesh> > lods;
        unsigned short nodeIdx;
        MultiResMeshData() : nodeIdx(0) {}
    };
    struct UserData {
        jet::String key;
        jet::String value;
    };
    struct MaterialData {
        jet::String name;
        jet::video::Material material;
        std::vector<jet::String> textureNames;
    };

    static const unsigned k_maxLodCount = 6;
    static unsigned s_hwSkinningBoneLimit;

    ModelBase();
    virtual ~ModelBase();

    Model *CreateInstance();
    void RemoveInstance(Model*) const;
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();
    virtual void Unload();

    unsigned GetTriangleCount() const { return m_triangleCount; }
    unsigned GetDips() const { return m_dips; }
    unsigned GetNodeCount() const { return (unsigned)m_nodes.size(); }
    const NodeData &GetNode(unsigned i) const { return m_nodes[i]; }
    int FindNodeIdxByName(const jet::String&) const;
    unsigned GetDummyCount() const { return (unsigned)m_dummies.size(); }
    const DummyData &GetDummy(unsigned i) const { return m_dummies[i]; }
    unsigned GetCameraCount() const { return (unsigned)m_cameras.size(); }
    const CameraData &GetCamera(unsigned i) const { return m_cameras[i]; }
    int FindCameraIdxByName(const jet::String&) const;
    unsigned GetMultiResMeshCount() const { return (unsigned)m_multiResMeshes.size(); }
    const MultiResMeshData &GetMultiResMesh(unsigned i) const { return m_multiResMeshes[i]; }
    unsigned GetMaxLod() const { return m_maxLod; }
    unsigned GetMaterialCount() const { return (unsigned)m_materials.size(); }
    const MaterialData &GetMaterialData(unsigned i) const { return m_materials[i]; }
    int FindMaterialIdxByName(const jet::String&) const;
    unsigned GetUserDataCount() const { return (unsigned)m_userData.size(); }
    const UserData &GetUserData(unsigned i) const { return m_userData[i]; }
    const UserData *FindUserDataByKey(const jet::String&) const;
    // ModelLoader::Params, set before Load (@0x701708..)
    void SetGeometryCPUAccess(bool v) { m_geometryCPUAccess = v; }
    void SetLoadRenderTechniques(bool v) { m_loadRenderTechniques = v; }
    void SetLoadTextures(bool v) { m_loadTextures = v; }

protected:
    MaterialData *AddMaterial(const jet::String&);
    void ReleaseMaterials();
    bool LoadV100(jet::stream::IStream&);
    bool LoadNodeV100(NodeData&, jet::stream::IStream&);
    bool LoadCameraV100(CameraData&, jet::stream::IStream&);
    bool LoadMultiResMeshV100(MultiResMeshData&, jet::stream::IStream&);
    void CollectMaterials(const Mesh&);

    mutable std::vector<Model*> m_instances;
    std::vector<MaterialData> m_materials;
    boost::unordered_map<jet::String, unsigned> m_nameMaterialIdxMap;
    std::vector<DummyData> m_dummies;
    std::vector<NodeData> m_nodes;
    boost::unordered_map<jet::String, unsigned> m_nameNodeIdxMap;
    boost::unordered_map<jet::String, unsigned> m_nameCameraIdxMap;
    std::vector<MultiResMeshData> m_multiResMeshes;
    unsigned m_maxLod;
    std::vector<UserData> m_userData;
    std::vector<CameraData> m_cameras;
    unsigned m_triangleCount;
    unsigned m_dips;
    bool m_geometryCPUAccess;     // 0x24
    bool m_loadRenderTechniques;  // 0x25
    bool m_loadTextures;          // 0x26
};
}

#endif
