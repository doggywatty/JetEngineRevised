#undef NDEBUG
#include "recorddb.h"
#include "stream/memorystream.h"
#include "stream/filestream.h"
#include <cassert>
#include <cmath>
#include <cstdio>

using namespace clara;

static void Fill(RecordDB& db)
{
    db.Set("coins", Record(1234));
    db.Set("name", Record(jet::String("Minion")));
    db.Set("big", Record(1LL << 40));
    db.Set("speed", Record(1.5f));
    db.Set("pi", Record(3.141592653589793));
    db.Set("on", Record(true));
    const u8 bin[] = {1, 2, 3, 250};
    db.Set("blob", Record(bin, 4));
    db.Set("costumes/goggles/owned", Record(true));
    db.Set("costumes/goggles/level", Record(3));
}

static void Check(const RecordDB& db, bool xml = false)
{
    assert(db.Get("coins").GetAsS32() == 1234 && db.Get("name").GetAsString() == "Minion");
    assert(db.Get("big").GetAsS64() == (1LL << 40) && db.Get("speed").GetAsFloat() == 1.5f);
    // XML stores doubles with %g, as the original
    assert((xml ? std::fabs(db.Get("pi").GetAsDouble() - 3.14159) < 1e-5 : db.Get("pi").GetAsDouble() == 3.141592653589793) && db.Get("on").GetAsBool());
    assert(db.Get("blob").GetAsBinary().size() == 4 && db.Get("blob").GetAsBinary()[3] == 250);
    assert(db.Get("costumes/goggles/level").GetAsS32() == 3 && db.Has("costumes/goggles/owned"));
    assert(!db.Has("costumes/nope") && db.Get("nope").GetType() == Record::TypeDef::NONE);
}

int main(int argc, char** argv)
{
    std::string dir = argc > 1 ? argv[1] : ".";
    RecordDB a;
    Fill(a);
    Check(a);
    assert(a.Set("coins", Record(1.0f)).IsReadOnly() && a.Get("coins").GetAsS32() == 1234);
    a.Increment("coins");
    assert(a.Get("coins").GetAsS32() == 1235);
    a.Decrement("coins");

    for (int mode = 0; mode < 4; ++mode) {
        RecordDB w;
        Fill(w);
        if (mode == 1) { w.SetEncryption(RecordDB::EncryptionDef::XTEA); w.SetEncryptionKey("secret"); }
        if (mode == 2) w.SetFormat(RecordDB::FormatDef::XML);
        if (mode == 3) w.SetRedundancyCount(3);
        jet::stream::MemoryStream m;
        w.Save(m);
        m.SeekBeg(0);
        RecordDB r;
        if (mode == 1) r.SetEncryptionKey("secret");
        if (mode == 3) r.SetRedundancyCount(3);
        assert(r.Load(m));
        Check(r, mode == 2);
        assert(mode == 2 || r == w);
        printf("mode %d: %u bytes\n", mode, m.Size());
        if (mode == 3) {
            std::vector<u8>& d = m.GetInternalArray();
            for (size_t i = 180; i < 260 && i < d.size(); ++i) d[i] ^= 0x5a;
            m.SeekBeg(0);
            RecordDB c;
            c.SetRedundancyCount(3);
            assert(c.Load(m));
            Check(c);
            puts("redundant copy survived corruption");
        }
    }
    RecordDB f;
    f.SetPath(jet::String(dir));
    f.SetName("save.dat");
    Fill(f);
    f.Save();
    RecordDB g;
    g.SetPath(jet::String(dir));
    g.SetName("save.dat");
    assert(g.Exists() && g.Load());
    Check(g);
    g.Delete();
    assert(!g.Exists());
    puts("recorddb ok");
}
