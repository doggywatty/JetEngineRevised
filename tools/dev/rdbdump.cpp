#include "recorddb.h"
#include "stream/filestream.h"
#include <cstdio>
using namespace clara;
static void Dump(const RecordDB& db, int d, int& n)
{
    db.ForEach([&](const jet::String& k, const Record& r) {
        if (n++ > 60) return;
        printf("%*s%s [%d] ", d * 2, "", k.c_str(), int(r.GetType().underlying()));
        switch (r.GetType()) {
        case Record::TypeDef::BOOL: printf("%d", r.GetAsBool()); break;
        case Record::TypeDef::S32: printf("%d", r.GetAsS32()); break;
        case Record::TypeDef::S64: printf("%lld", r.GetAsS64()); break;
        case Record::TypeDef::FLOAT: printf("%g", r.GetAsFloat()); break;
        case Record::TypeDef::DOUBLE: printf("%g", r.GetAsDouble()); break;
        case Record::TypeDef::STRING: printf("'%s'", r.GetAsCString().c_str()); break;
        case Record::TypeDef::BINARY: printf("%zu bytes", r.GetAsBinary().size()); break;
        default: break;
        }
        printf("\n");
        if (r.GetAsRecordDB()) Dump(*r.GetAsRecordDB(), d + 1, n);
    });
}
int main(int argc, char** argv)
{
    RecordDB db;
    if (argc > 2) db.SetRedundancyCount(unsigned(atoi(argv[2])));
    if (argc > 3) db.SetEncryptionKey(argv[3]);
    jet::stream::FileStream f(jet::String(argv[1]), jet::stream::FileStream::k_in, false);
    bool ok = db.Load(f);
    int n = 0;
    printf("load %s: %d records (format %d)\n", ok ? "ok" : "FAILED", db.GetCount(), int(db.GetFormat().underlying()));
    Dump(db, 0, n);
}
