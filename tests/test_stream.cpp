#undef NDEBUG
#include "core/pstring.h"
#include "stream/memlinkstream.h"
#include "stream/substream.h"
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace jet;
using namespace jet::stream;

int main()
{
    MemoryStream m;
    u32 a = 0xdeadbeef;
    float f = 1.5f;
    m << a << f << String("Minion") << std::string("Rush");
    assert(m.Size() == 4 + 4 + 8 + 6);
    m.SeekBeg(0);
    u32 a2;
    float f2;
    String s;
    std::string t;
    m >> a2 >> f2 >> s >> t;
    assert(a2 == a && f2 == f && s.EqualsCS("Minion") && t == "Rush" && m.Tell() == m.Size());
    u32 z = 1;
    assert(m.Read4(&z) == 4 && z == 0);

    u8 raw[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    MemLinkStream l(raw, 8);
    SubStream sub;
    sub.SetSource(l, 2, 4);
    sub.Start();
    u8 out[8] = {};
    assert(sub.Read(out, 8) == 4 && memcmp(out, raw + 2, 4) == 0 && sub.IsStarted());
    sub.Finish();
    assert(!l.IsStarted());
    const u8 ro[2] = {9, 9};
    MemLinkStream r(ro, 2);
    assert(r.Write(raw, 1) == 0);

    assert(m.RttiIsOfType(IStream::RttiGetClassId()) && !m.RttiIsOfType(SubStream::RttiGetClassId()));
    assert(l.RttiCastTo(MemoryStream::RttiGetClassId()) == &l && strcmp(l.RttiGetInstanceClassIdName(), "MemLinkStream") == 0);
    puts("stream ok");
}
