#undef NDEBUG
#include "project.h"
#include "stream/filestream.h"
#include <cassert>
#include <cstdio>

using namespace clara;

static void Dump(const Folder& f, int depth, unsigned& n)
{
    for (unsigned i = 0; i < f.GetEntityCount(); ++i, ++n)
        if (depth < 2 && i < 3) {
            DataEntity* e = f.GetEntity(i);
            printf("%*s%s (%s, %u params)\n", depth * 2, "", e->GetName().c_str(), e->GetTemplateName().c_str(), e->GetParamCount());
        }
    for (unsigned i = 0; i < f.GetFolderCount(); ++i)
        Dump(f.GetFolder(i), depth + 1, n);
}

int main(int argc, char** argv)
{
    const char* dir = argc > 1 ? argv[1] : ".";
    Project p;
    DefaultCreator c;
    std::string d = dir;
    jet::stream::FileStream lib(jet::String(d + "/" + (argc > 2 ? argv[2] : "designlib.blibclara")), jet::stream::FileStream::k_in, false);
    lib.Start();
    p.LoadLibraries(lib, c);
    printf("types %u templates %u libraries %u entities %u, read %u of %u\n", p.GetTypeCount(), p.GetTemplateCount(), p.GetLibraryCount(),
           p.GetEntityCount(), lib.Tell(), lib.Size());
    assert(lib.Tell() == lib.Size());
    lib.Finish();
    for (const char* lv : {"level_empty.bclara", "intro.bclara", "level.bclara"}) {
        jet::stream::FileStream s(jet::String(d + "/" + lv), jet::stream::FileStream::k_in, false);
        s.Start();
        p.LoadClaraFile(s, c);
        unsigned n = 0;
        printf("%s: '%s' version 0x%x, read %u of %u\n", lv, p.GetClaraFile()->GetName().c_str(), p.GetClaraFile()->GetVersion(), s.Tell(), s.Size());
        Dump(p.GetClaraFile()->GetRoot(), 0, n);
        printf("  %u entities, project total %u\n", n, p.GetEntityCount());
        assert(s.Tell() == s.Size());
        s.Finish();
    }
    p.Init();
    p.PostInit();
    puts("clara ok");
}
