#undef NDEBUG
#include "encryption.h"
#include "stream/filestream.h"
#include "stream/istreamfactory.h"
#include "stream/streammgr.h"
#include "stream/streamutil.h"
#include "stream/zipfilesystem.h"
#include <zlib.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

using namespace jet;
using namespace jet::stream;

static std::string ReadAll(IStream* s)
{
    assert(s);
    s->Start();
    std::string r(s->Size(), '\0');
    if (!r.empty())
        s->Read(&r[0], unsigned(r.size()));
    s->Finish();
    delete s;
    return r;
}

static void Put(const std::string& path, const std::string& data)
{
    FILE* f = fopen(path.c_str(), "wb");
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);
}

// A zip archive written here, so the test does not depend on a file that is not in the repository
struct ZipItem {
    std::string name, data;
    bool deflate;
};

static void Le16(std::string& s, unsigned v) { s += char(v & 0xFF), s += char(v >> 8); }
static void Le32(std::string& s, unsigned v) { Le16(s, v & 0xFFFF), Le16(s, v >> 16); }

static std::string Deflate(const std::string& in)  // raw deflate (no zlib header), as zip stores it
{
    z_stream zs = {};
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    std::string out(deflateBound(&zs, uLong(in.size())), '\0');
    zs.next_in = (Bytef*)in.data();
    zs.avail_in = uInt(in.size());
    zs.next_out = (Bytef*)&out[0];
    zs.avail_out = uInt(out.size());
    deflate(&zs, Z_FINISH);
    out.resize(zs.total_out);
    deflateEnd(&zs);
    return out;
}

static std::string MakeZip(const std::vector<ZipItem>& items)
{
    std::string body, central;
    for (const ZipItem& it : items) {
        std::string packed = it.deflate ? Deflate(it.data) : it.data;
        unsigned method = it.deflate ? 8 : 0;
        unsigned crc = unsigned(crc32(0, (const Bytef*)it.data.data(), uInt(it.data.size())));
        unsigned offset = unsigned(body.size());
        const unsigned date = 0x21;  // 1980-01-01, time 00:00
        Le32(body, 0x04034b50), Le16(body, 20), Le16(body, 0), Le16(body, method), Le16(body, 0), Le16(body, date);
        Le32(body, crc), Le32(body, unsigned(packed.size())), Le32(body, unsigned(it.data.size()));
        Le16(body, unsigned(it.name.size())), Le16(body, 0);
        body += it.name + packed;
        Le32(central, 0x02014b50), Le16(central, 20), Le16(central, 20), Le16(central, 0), Le16(central, method);
        Le16(central, 0), Le16(central, date);
        Le32(central, crc), Le32(central, unsigned(packed.size())), Le32(central, unsigned(it.data.size()));
        Le16(central, unsigned(it.name.size())), Le16(central, 0), Le16(central, 0), Le16(central, 0), Le16(central, 0);
        Le32(central, 0), Le32(central, offset);
        central += it.name;
    }
    std::string end;
    Le32(end, 0x06054b50), Le16(end, 0), Le16(end, 0), Le16(end, unsigned(items.size())), Le16(end, unsigned(items.size()));
    Le32(end, unsigned(central.size())), Le32(end, unsigned(body.size())), Le16(end, 0);
    return body + central + end;
}

int main(int argc, char** argv)
{
    std::string dir = argc > 1 ? argv[1] : ".";
    std::string d = dir + "/data";
    MakeDirectory(String(d));
    Put(d + "/hello.txt", "hello from disk");

    // .lib: u32 count; u32 offsets; names; padding; data
    std::string a = "AAAA", b = "second entry";
    std::string names;
    for (std::string n : {std::string("a.bin"), std::string("b.txt")})
        names += char(n.size()), names += '\0', names += n;
    u32 head = 4 + 8 + u32(names.size()) + 2;
    u32 offs[2] = {head, head + u32(a.size()) + 1};
    std::string lib((char*)"\x02\0\0\0", 4);
    lib.append((char*)offs, 8);
    lib += names;
    lib += std::string("\x01\x00", 2);
    lib += a + "!" + b;
    Put(dir + "/pack.lib", lib);

    // the zip the checks at the end read back: a stored file, a deflated file and a file in a folder
    Put(dir + "/test.zip", MakeZip({ { "stored.txt", "stored in zip", false },
                                     { "deflated.txt", std::string(30000, 'z'), true },
                                     { "sub/dir/inner.txt", "nested", false } }));

    // encrypted: u16 1, XTEA([u32 size][u32 crc][data])
    std::string secret = "top secret payload";
    std::string plain(8, '\0');
    u32 sz = u32(secret.size()), crc = ComputeCRC((const u8*)secret.data(), sz);
    memcpy(&plain[0], &sz, 4);
    memcpy(&plain[4], &crc, 4);
    plain += secret;
    std::vector<u32> enc;
    encryption::EncipherXTEA((const u8*)plain.data(), unsigned(plain.size()), String("key123"), enc);
    Put(d + "/secret.dat.enc", std::string("\x01\x00", 2) + std::string((char*)enc.data(), enc.size() * 4));

    StreamMgr& m = StreamMgr::GetInstance();
    assert(m.AddPath(String(d), String()));
    assert(m.AddPath(String(dir + "/pack.lib"), String()));
    assert(m.AddPath(String(dir + "/test.zip"), String()));
    m.AddEncryptionPair(String("key123"), String(".enc"));

    assert(ReadAll(m.CreateStream("HELLO.txt")) == "hello from disk");
    assert(ReadAll(m.CreateStream("a.bin")) == "AAAA" && ReadAll(m.CreateStream("b.txt")) == b);
    assert(ReadAll(m.CreateStream("secret.dat")) == secret);
    assert(ReadAll(m.CreateStream("stored.txt")) == "stored in zip");
    std::string big = ReadAll(m.CreateStream("deflated.txt"));
    assert(big.size() == 30000 && big.find_first_not_of('z') == std::string::npos);
    assert(!m.CanCreateStream("nope.bin") && m.CanCreateStream("secret.dat") && !m.CanCreateStream("secret.dat", false));

    ZipFileSystem zfs(String(dir + "/test.zip"));
    const IFileSystem::Entry& e = zfs.FindEntryByPath(0, "sub/dir/inner.txt");
    assert(e && zfs.GetEntryFullPath(unsigned(e.id)) == "sub/dir/inner.txt");
    assert(ReadAll(zfs.CreateStream(unsigned(e.id))) == "nested");
    assert(zfs.FindEntryByName(0, "INNER.TXT", true).id == e.id && !zfs.FindEntryByName(0, "inner.txt", false));
    StreamMgr::FreeInstance();
    puts("streammgr ok");
}
