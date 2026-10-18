// Prints the libraries and levels of a data dir: library names, folder trees and template counts.
//   test_clara_tree <data dir> [max depth]
#include "project.h"
#include "folder.h"
#include "type.h"
#include "stream/filestream.h"
#include <cstdio>
#include <cstring>
#include "entity.h"
#include <map>
#include <string>

using namespace clara;

static const char *s_find = 0;
static void PrintParams(const DataEntity *e, int indent) {
    for (unsigned k = 0; k < e->GetParamCount(); k++) {
        const Param *p = e->GetParam(k);
        printf("%*s%s =", indent, "", p->GetName().c_str());
        const unsigned vt = p->GetType() ? p->GetType()->GetValueType() : 0;
        for (unsigned c = 0; c < p->GetComponentCount() && c < 8; c++) {
            if (vt == Type::SCALAR) printf(" %g", p->GetAsDouble(c));
            else if (vt == Type::ENUM) printf(" %s", p->GetAsEnumName(c).c_str());
            else if (vt == Type::NODE_REF) printf(" %s", p->GetAsPath(c).GetAsString().c_str());
            else if (vt == Type::ENTITY) printf(" <entity %s>", p->GetAsEntity(c) ? p->GetAsEntity(c)->GetTemplateName().c_str() : "null");
            else if (vt == Type::STATE) printf(" %s|%s", p->GetAsState(c).a.c_str(), p->GetAsState(c).b.c_str());
            else if (vt == Type::STRING) printf(" '%s'", p->GetAsString(c).c_str());
            else printf(" <%u>", vt);
        }
        printf("\n");
        if (vt == Type::ENTITY)
            for (unsigned c = 0; c < p->GetComponentCount() && c < 8; c++)
                if (p->GetAsEntity(c)) PrintParams(p->GetAsEntity(c), indent + 4);
    }
}

static const char *s_refs = 0;
static void FindRefs(const DataEntity *de, const std::string &where, int depth) {
    if (depth > 6) return;
    for (unsigned k = 0; k < de->GetParamCount(); k++) {
        const Param *p = de->GetParam(k);
        const unsigned vt = p->GetType() ? p->GetType()->GetValueType() : 0;
        for (unsigned c = 0; c < p->GetComponentCount(); c++) {
            if (vt == Type::ENTITY) { if (DataEntity *sub = p->GetAsEntity(c)) FindRefs(sub, where, depth + 1); }
            if (vt != Type::NODE_REF && vt != Type::ENTITY) continue;
            if (strstr(p->GetAsPath(c).GetAsString().c_str(), s_refs)) {
                const Entity *e = dynamic_cast<const Entity*>(de);
                printf("REF %s [%s] %s . %s[%u] = %s (depth %d)\n", where.c_str(), de->GetTemplateName().c_str(), e ? e->GetName().c_str() : "?", p->GetName().c_str(), c, p->GetAsPath(c).GetAsString().c_str(), depth);
            }
        }
    }
}

static void Find(const Folder &f, const std::string &path) {
    std::string me = path + "/" + f.GetName().c_str();
    for (unsigned i = 0; i < f.GetEntityCount(); i++) {
        const DataEntity *de = f.GetEntity(i);
        const Entity *e = dynamic_cast<const Entity*>(de);
        if (e && strstr(e->GetName().c_str(), s_find)) {
            math::vec3<float> pos = e->GetPosition();
            printf("FOUND %s [%s] %s (%.2f %.2f %.2f)\n", me.c_str(), e->GetTemplateName().c_str(), e->GetName().c_str(), pos.x, pos.y, pos.z);
            // PARAMS=1 lists the parameters of what was found: name = components (scalar, enum, path, string, state), the entities the
            // parameters hold below them
            if (getenv("PARAMS")) PrintParams(de, 4);
        }
    }
    // REFS=text lists the entities (and the entities their parameters hold) with a path parameter that holds the text
    if (s_refs)
        for (unsigned i = 0; i < f.GetEntityCount(); i++) FindRefs(f.GetEntity(i), me, 0);
    for (unsigned i = 0; i < f.GetFolderCount(); i++) Find(f.GetFolder(i), me);
}

static void Dump(const Folder &f, int depth, int maxDepth) {
    std::map<std::string, int> counts;
    for (unsigned i = 0; i < f.GetEntityCount(); i++) counts[f.GetEntity(i)->GetTemplateName().c_str()]++;
    printf("%*s[%s]", depth * 2, "", f.GetName().c_str());
    for (auto &c : counts) printf(" %s x%d", c.first.c_str(), c.second);
    printf("\n");
    if (depth < maxDepth)
        for (unsigned i = 0; i < f.GetFolderCount(); i++) Dump(f.GetFolder(i), depth + 1, maxDepth);
}

int main(int argc, char **argv) {
    std::string d = argc > 1 ? argv[1] : ".";
    int maxDepth = argc > 2 ? atoi(argv[2]) : 2;
    Project p;
    DefaultCreator c;
    for (const char *l : {"designlib.blibclara", "lablib.blibclara", "macholib.blibclara"}) {
        jet::stream::FileStream s(jet::String((d + "/levels/" + l).c_str()), jet::stream::FileStream::k_in, false);
        s.Start();
        p.AddLibraries(s, c);
        s.Finish();
    }
    if (getenv("TYPES"))
        for (unsigned i = 0; i < p.GetTypeCount(); i++) {
            const Type *t = p.GetType(i);
            if (!t->GetEnumCount()) continue;
            std::map<unsigned short, std::string> byValue;
            for (auto &e : t->GetEnumValues()) byValue[e.second] = e.first.c_str();
            printf("TYPE %s:", t->GetName().c_str());
            for (auto &e : byValue) printf(" %u=%s", e.first, e.second.c_str());
            printf("\n");
        }
    for (unsigned i = 0; i < p.GetLibraryCount(); i++) {
        printf("LIB '%s'\n", p.GetLibrary(i)->GetName().c_str());
        Dump(p.GetLibrary(i)->GetRoot(), 1, maxDepth);
    }
    for (const char *lv : {"level.bclara"}) {
        jet::stream::FileStream s(jet::String((d + "/levels/" + lv).c_str()), jet::stream::FileStream::k_in, false);
        s.Start();
        p.LoadClaraFile(s, c);
        printf("LEVEL '%s'\n", p.GetClaraFile()->GetName().c_str());
        s_refs = getenv("REFS");
        if (getenv("FIND")) { s_find = getenv("FIND"); Find(p.GetClaraFile()->GetRoot(), ""); for (unsigned i = 0; i < p.GetLibraryCount(); i++) Find(p.GetLibrary(i)->GetRoot(), "LIB"); }
        else Dump(p.GetClaraFile()->GetRoot(), 1, maxDepth + 2);
        s.Finish();
    }
}
