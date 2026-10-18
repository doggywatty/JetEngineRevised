#undef NDEBUG
// Loads every .pig in a directory through ModelBase and checks the structure: counts, and that
// each submesh's vertex/index blob decodes to in-range indices (needs a GL context for Geometry).
#include "scene/modelbase.h"
#include "scene/model.h"
#include "scene/mesh.h"
#include "scene/submesh.h"
#include "stream/filestream.h"
#include "video/driver.h"
#include "video/gles20/gles20driver.h"
#if defined(__APPLE__)
#include "video/mac/macdisplay.h"
typedef jet::video::MacDisplay TestDisplay;
#else
#include "testdisplay.h"
#endif
#include <cassert>
#include <cstdio>
#include <dirent.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace jet;

int main(int argc, char **argv) {
    std::string dir = argc > 1 ? argv[1] : "models";
    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(16, 16);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));

    std::vector<std::string> files;
    if (DIR *d = opendir(dir.c_str())) {
        while (dirent *e = readdir(d)) {
            std::string n = e->d_name;
            if (n.size() > 4 && n.substr(n.size() - 4) == ".pig") files.push_back(n);
        }
        closedir(d);
    }
    std::sort(files.begin(), files.end());
    unsigned models = 0, submeshes = 0, tris = 0;
    for (const std::string &f : files) {
        stream::FileStream *s = new stream::FileStream(String((dir + "/" + f).c_str()), stream::FileStream::k_in, false);
        boost::shared_ptr<scene::ModelBase> base(new scene::ModelBase());
        base->SetResourceName(String(f.c_str()));
        base->Init(s);
        assert(base->Load());
        assert(s->Tell() == s->Size());
        for (unsigned m = 0; m < base->GetMultiResMeshCount(); m++)
            for (unsigned l = 0; l < scene::ModelBase::k_maxLodCount; l++) {
                boost::shared_ptr<scene::Mesh> mesh = base->GetMultiResMesh(m).lods[l];
                if (!mesh) continue;
                for (unsigned i = 0; i < mesh->GetSubMeshCount(); i++) {
                    const scene::SubMesh &sm = mesh->GetSubMesh(i);
                    boost::shared_ptr<video::Geometry> g = sm.GetGeometry();
                    assert(g && g->GetVertexCount() == sm.GetVertexCount() && g->GetIndexCount() == sm.GetIndexCount());
                    const unsigned short *idx = g->MapIndicesRead();
                    for (unsigned k = 0; k < sm.GetIndexCount(); k++) assert(idx[k] < sm.GetVertexCount());
                    ++submeshes;
                }
            }
        scene::Model *inst = base->CreateInstance();
        delete inst;
        tris += base->GetTriangleCount();
        ++models;
        if (argc > 2 && f == argv[2]) {
            printf("%s: nodes %u cams %u meshes %u materials %u tris %u maxlod %u\n", f.c_str(), base->GetNodeCount(),
                   base->GetCameraCount(), base->GetMultiResMeshCount(), base->GetMaterialCount(), base->GetTriangleCount(), base->GetMaxLod());
            for (unsigned i = 0; i < base->GetMaterialCount(); i++) {
                printf("  material %s:", base->GetMaterialData(i).name.c_str());
                for (auto &t : base->GetMaterialData(i).textureNames) printf(" %s", t.c_str());
                printf("\n");
            }
        }
    }
    printf("models %u submeshes %u triangles %u\n", models, submeshes, tris);
    display.Cleanup();
    puts("model ok");
}
