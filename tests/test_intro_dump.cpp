#undef NDEBUG
// Dumps the entity tree, groups and movies of a level file: test_intro_dump <data dir> [level]
#include "project.h"
#include "param.h"
#include "type.h"
#include "folder.h"
#include "stream/streammgr.h"
#include "stream/filestream.h"
#include "game/entities/GameEntityCreator.h"
#include <cstdio>
#include "game/entities/Deco3d.h"
#include <string>

static void Dump(const clara::Folder &f, int depth) {
    std::string pad(depth * 2, ' ');
    printf("%s[%s]\n", pad.c_str(), f.GetName().c_str());
    for (unsigned i = 0; i < f.GetEntityCount(); i++) {
        clara::DataEntity *e = f.GetEntity(i);
        printf("%s  %s : %s", pad.c_str(), e->GetName().c_str(), e->GetTemplateName().c_str());
        if (clara::Entity *en = dynamic_cast<clara::Entity*>(e)) printf(" pos(%g %g %g)", en->GetPosition().x, en->GetPosition().y, en->GetPosition().z);
        if (dynamic_cast<Deco3d*>(e)) { jet::String m; e->GetParam(jet::String("Model"), m); printf(" model=%s", m.c_str()); }
        printf("\n");
    }
    for (unsigned i = 0; i < f.GetEntityCount(); i++) {
        clara::DataEntity *e = f.GetEntity(i);
        if (e->GetTemplateName().Equals("LevelDef"))
            for (unsigned p = 0; p < e->GetParamCount(); p++) {
                clara::Param *pa = e->GetParam(p);
                if (pa->GetType() && pa->GetType()->GetValueType() == clara::Type::SCRIPT && pa->GetComponentCount())
                    printf("%s  SCRIPT %s:\n%s\n", pad.c_str(), pa->GetName().c_str(), pa->GetAsScript(0).c_str());
            }
    }
    for (unsigned i = 0; i < f.GetGroupCount(); i++) printf("%s  group %s (%u)\n", pad.c_str(), f.GetGroup(i).GetName().c_str(), f.GetGroup(i).GetPathCount());
    for (unsigned i = 0; i < f.GetMovieCount(); i++) {
        clara::Movie &m = f.GetMovie(i);
        printf("%s  movie %s %d..%d fps %u tracks %u\n", pad.c_str(), m.GetName().c_str(), m.GetStart(), m.GetEnd(), m.GetFps(), m.GetTrackCount());
        for (unsigned t = 0; t < m.GetTrackCount(); t++) {
            const clara::Movie::TrackData &d = m.GetTrack(t);
            printf("%s    track %u type %d pos %zu rot %zu scale %zu anim %zu script %zu\n", pad.c_str(), t, (int)d.type, d.position.size(), d.rotation.size(), d.scale.size(), d.animation.size(), d.script.size());
            for (const auto &a : d.animation) printf("%s      anim t=%d %s start=%u loops=%u playing=%d\n", pad.c_str(), a.time, a.name.c_str(), a.start, a.loops, a.playing);
            for (const auto &a : d.script) printf("%s      script t=%d %s(%s)\n", pad.c_str(), a.time, a.function.c_str(), a.args.c_str());
        }
    }
    for (unsigned i = 0; i < f.GetFolderCount(); i++) Dump(f.GetFolder(i), depth + 1);
}

int main(int argc, char **argv) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string level = argc > 2 ? argv[2] : "intro";
    clara::Project project;
    GameEntityCreator creator;
    for (const char *lib : { "designlib", "lablib" }) {
        jet::stream::FileStream s(jet::String((data + "/levels/" + lib + ".blibclara").c_str()), jet::stream::FileStream::k_in, false);
        project.AddLibraries(s, creator);
    }
    jet::stream::FileStream lvl(jet::String((data + "/levels/" + level + ".bclara").c_str()), jet::stream::FileStream::k_in, false);
    lvl.Start();
    project.LoadClaraFile(lvl, creator);
    lvl.Finish();
    Dump(project.GetClaraFile()->GetRoot(), 0);
    return 0;
}
