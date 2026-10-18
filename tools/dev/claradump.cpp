// claradump <levels dir> <file.bclara> [library]: every entity of a clara file with its parameters.
#include "project.h"
#include "group.h"
#include "movie.h"
#include "folder.h"
#include "stream/filestream.h"
#include <cstdio>
#include <string>

using namespace clara;

static void Value(const Param& p, unsigned i)
{
    const Type* t = p.GetType();
    switch (t ? t->GetValueType() : 0) {
    case Type::SCALAR: printf("%g", p.GetAsDouble(i)); break;
    case Type::ENUM: printf("%s", p.GetAsEnumName(i).c_str()); break;
    case Type::NODE_REF: printf("node:%s", p.GetAsPath(i).GetAsString().c_str()); break;
    case Type::SCRIPT: printf("script:\"%s\"", p.GetAsScript(i).c_str()); break;
    case Type::ENTITY: {
        DataEntity* e = p.GetAsEntity(i);
        printf("-> %s", e ? e->GetName().c_str() : "null");
        if (e) {  // inline sub-entities (material passes etc.)
            printf(" [%s] {", e->GetTemplateName().c_str());
            for (unsigned k = 0; k < e->GetParamCount(); ++k) {
                const Param* q = e->GetParam(k);
                printf(" %s=", q->GetName().c_str());
                for (unsigned c = 0; c < q->GetComponentCount(); ++c) { if (c) printf(","); Value(*q, c); }
            }
            printf(" }");
        }
        break;
    }
    case Type::STRING: printf("\"%s\"", p.GetAsString(i).c_str()); break;
    case Type::VALUE: {
        unsigned n = p.GetValueSize(i);
        const float* f = p.GetAsFloats(i);
        printf("(");
        for (unsigned k = 0; k < n; ++k) printf(k ? ", %g" : "%g", f[k]);
        printf(")");
        break;
    }
    case Type::SPRITE: printf("sprite"); break;
    case Type::STATE: printf("state(%s | %s)", p.GetAsState(i).a.c_str(), p.GetAsState(i).b.c_str()); break;
    default: printf("<type %u>", t ? t->GetValueType() : 0);
    }
}

static void Dump(const Folder& f, int depth)
{
    for (unsigned i = 0; i < f.GetEntityCount(); ++i) {
        DataEntity* e = f.GetEntity(i);
        printf("%*s[%s] %s id=%u", depth * 2, "", e->GetTemplateName().c_str(), e->GetName().c_str(), e->GetId());
        if (Entity* en = dynamic_cast<Entity*>(e)) {
            math::vec3<float> p = en->GetPosition(), s = en->GetScale();
            math::quat<float> q = en->GetRotation();
            printf(" pos(%g %g %g) rot(%g %g %g %g) scale(%g %g %g)", p.x, p.y, p.z, q.x, q.y, q.z, q.w, s.x, s.y, s.z);
            if (en->GetLinkee()) printf(" linked-to %s@%s", en->GetLinkee()->GetName().c_str(), en->GetLinkPointName().c_str());
            if (en->GetAnimation()) printf(" anim=%s", en->GetAnimationName().c_str());
        }
        printf("\n");
        for (unsigned k = 0; k < e->GetParamCount(); ++k) {
            const Param* p = e->GetParam(k);
            printf("%*s  %s =", depth * 2, "", p->GetName().c_str());
            for (unsigned c = 0; c < p->GetComponentCount(); ++c) {
                printf(" ");
                if (p->GetComponentCount() > 1 && c < 64) printf("[%s]", p->GetComponentName(c).c_str());
                Value(*p, c);
            }
            printf("\n");
        }
    }
    for (unsigned i = 0; i < f.GetGroupCount(); ++i) {
        Group& g = f.GetGroup(i);
        printf("%*s<group %s>", depth * 2, "", g.GetName().c_str());
        for (unsigned k = 0; k < g.GetPathCount(); ++k) printf(" %s", g.GetPath(k).GetAsString().c_str());
        printf("\n");
    }
    for (unsigned i = 0; i < f.GetMovieCount(); ++i) {
        Movie& m = f.GetMovie(i);
        printf("%*s<movie %s fps=%u start=%d end=%d loop=%d tracks=%u>\n", depth * 2, "", m.GetName().c_str(), m.GetFps(), m.GetStart(), m.GetEnd(), m.IsLooping(), m.GetTrackCount());
        for (unsigned t = 0; t < m.GetTrackCount(); ++t) {
            const Movie::TrackData& d = m.GetTrack(t);
            printf("%*s  track %d %s pos=%zu rot=%zu scale=%zu anim=%zu script=%zu\n", depth * 2, "", d.type, d.path.GetAsString().c_str(),
                   d.position.size(), d.rotation.size(), d.scale.size(), d.animation.size(), d.script.size());
            for (auto& a : d.animation) printf("%*s    anim t=%d %s start=%u loops=%u playing=%d\n", depth * 2, "", a.time, a.name.c_str(), a.start, a.loops, a.playing);
            for (auto& a : d.script) printf("%*s    script t=%d %s(%s)\n", depth * 2, "", a.time, a.function.c_str(), a.args.c_str());
            for (auto& a : d.position) printf("%*s    pos t=%d i=%u (%g %g %g)\n", depth * 2, "", a.time, a.interp, a.value.x, a.value.y, a.value.z);
        }
    }
    for (unsigned i = 0; i < f.GetFolderCount(); ++i) {
        printf("%*s{folder %s}\n", depth * 2, "", f.GetFolder(i).GetName().c_str());
        Dump(f.GetFolder(i), depth + 1);
    }
}

int main(int argc, char** argv)
{
    std::string d = argv[1];
    Project p;
    DefaultCreator c;
    jet::stream::FileStream lib(jet::String(d + "/" + (argc > 3 ? argv[3] : "designlib.blibclara")), jet::stream::FileStream::k_in, false);
    lib.Start();
    p.LoadLibraries(lib, c);
    lib.Finish();
    if (std::string(argv[2]) == "-lib") {  // dump the libraries' own folders
        for (unsigned i = 0; i < p.GetLibraryCount(); i++) {
            printf("=== library %s\n", p.GetLibrary(i)->GetName().c_str());
            Dump(p.GetLibrary(i)->GetRoot(), 0);
        }
        return 0;
    }
    jet::stream::FileStream s(jet::String(d + "/" + argv[2]), jet::stream::FileStream::k_in, false);
    s.Start();
    p.LoadClaraFile(s, c);
    s.Finish();
    Dump(p.GetClaraFile()->GetRoot(), 0);
}
