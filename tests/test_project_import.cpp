// Creates a JetEditor project, imports three pig models from data/models, one TGA made here, one PVR3 from
// data/textures_android and one shader from data/shaders, then checks the refusals (unsupported type, damaged
// file, duplicate) leave the project unchanged. The project is reopened and the database entries, the file
// hashes, the manifest and the database bytes are checked.
//   test_project_import [data dir] [work dir]    (defaults: data, build/project_import_work)
#include "editor/formats/pig_reader.h"
#include "editor/formats/pig_writer.h"
#include "editor/formats/tga.h"
#include "editor/project/importer.h"
#include "editor/project/project.h"
#include "editor/project/sha256.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

using namespace jeteditor;
namespace fs = std::filesystem;

namespace {

int g_checks = 0;
int g_failed = 0;

void Check(bool ok, const std::string &what) {
    g_checks++;
    if (!ok) g_failed++;
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
}

std::vector<std::string> ListSorted(const std::string &dir, const std::string &ext) {
    std::vector<std::string> out;
    std::error_code ec;
    for (const fs::directory_entry &e : fs::directory_iterator(dir, ec))
        if (e.is_regular_file(ec) && e.path().extension() == ext) out.push_back(e.path().string());
    std::sort(out.begin(), out.end());
    return out;
}

bool Read(const std::string &path, std::vector<uint8_t> &bytes) {
    std::string error;
    return project::ReadFile(path, bytes, error);
}

bool Write(const std::string &path, const std::vector<uint8_t> &bytes) {
    std::string error;
    return project::WriteFile(path, bytes, error);
}

// An uncompressed 32-bit TGA, written out by hand here (not by the code under test): 4 x 3 pixels, bottom-left origin,
// image ID "ABC". Pixel (x, y) is B = 40x, G = 70y, R = 100, A = 255 - x, rows stored bottom row first.
std::vector<uint8_t> MakeTga32() {
    std::vector<uint8_t> t = {3, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 0, 32, 0x08};
    t.push_back('A');
    t.push_back('B');
    t.push_back('C');
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 4; x++) {
            t.push_back(static_cast<uint8_t>(40 * x));
            t.push_back(static_cast<uint8_t>(70 * y));
            t.push_back(100);
            t.push_back(static_cast<uint8_t>(255 - x));
        }
    return t;
}

}  // namespace

int main(int argc, char **argv) {
    const std::string data = argc > 1 ? argv[1] : "data";
    const std::string work = argc > 2 ? argv[2] : "build/project_import_work";
    std::error_code ec;
    fs::remove_all(work, ec);
    fs::create_directories(work + "/inputs/rejected", ec);
    const std::string root = work + "/ImportTest";
    std::string error;

    // SHA-256 known answers: empty, one block, two blocks, and a million 'a'
    Check(project::Sha256Hex(nullptr, 0) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "sha256 of the empty message");
    const std::string abc = "abc";
    Check(project::Sha256Hex(reinterpret_cast<const uint8_t *>(abc.data()), abc.size()) ==
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
          "sha256 of abc");
    const std::string two = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    Check(project::Sha256Hex(reinterpret_cast<const uint8_t *>(two.data()), two.size()) ==
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
          "sha256 of the 56-byte two-block message");
    const std::vector<uint8_t> million(1000000, 'a');
    Check(project::Sha256Hex(million.data(), million.size()) ==
              "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
          "sha256 of one million 'a'");

    // Create
    project::Project p;
    const bool created = project::CreateProject(root, "ImportTest", p, error);
    Check(created, "create project" + (created ? std::string() : ": " + error));
    for (const char *folder : project::kLayoutFolders)
        Check(fs::is_directory(fs::path(root) / folder, ec), std::string("layout folder ") + folder);
    Check(fs::is_regular_file(fs::path(root) / "project.jetproj", ec), "manifest project.jetproj written");
    project::Project again;
    Check(!project::CreateProject(root, "Again", again, error), "second create in the same folder is refused");

    // Imports
    const std::vector<std::string> pigs = ListSorted(data + "/models", ".pig");
    const std::vector<std::string> shaders = ListSorted(data + "/shaders", ".xml");
    const std::vector<std::string> pvrs = ListSorted(data + "/textures_android", ".tga");
    Check(pigs.size() >= 3 && !shaders.empty() && !pvrs.empty(), "data dir has pig models, shaders and PVR textures");
    if (pigs.size() < 3 || shaders.empty() || pvrs.empty()) {
        std::printf("%d of %d checks failed\n", g_failed, g_checks);
        return 1;
    }

    for (size_t i = 0; i < 3; i++) {
        bool ok = project::ImportAsset(p, pigs[i], error);
        Check(ok, "import pig " + fs::path(pigs[i]).filename().string() + (ok ? "" : ": " + error));
    }
    const std::string tgaSource = work + "/inputs/made_32.tga";
    Check(Write(tgaSource, MakeTga32()), "write the test TGA");
    {
        bool ok = project::ImportAsset(p, tgaSource, error);
        Check(ok, "import TGA made_32.tga" + (ok ? std::string() : ": " + error));
    }
    {
        bool ok = project::ImportAsset(p, shaders[0], error);
        Check(ok, "import shader " + fs::path(shaders[0]).filename().string() + (ok ? "" : ": " + error));
    }
    {
        bool ok = project::ImportAsset(p, pvrs[0], error);
        Check(ok, "import PVR3 " + fs::path(pvrs[0]).filename().string() + (ok ? "" : ": " + error));
    }
    const size_t imported = p.assets.size();
    Check(imported == 6, "six assets in the database after the imports");

    // Refusals: each leaves the database and the folders as they were
    std::vector<uint8_t> pigBytes;
    Check(Read(pigs[0], pigBytes), "read a pig for the damaged-copy case");
    std::vector<uint8_t> tga = MakeTga32();

    std::vector<uint8_t> rle = tga;
    rle[2] = 10;  // RLE truecolor
    std::vector<uint8_t> footer = tga;
    footer.insert(footer.end(), 26, 0);  // bytes after the pixel data
    std::vector<uint8_t> colourMapped = tga;
    colourMapped[1] = 1;
    std::vector<uint8_t> pvrNotPvr = {'n', 'o', 'p', 'e'};
    const std::string badXml = "<technique><pass></technique>";
    const std::string wrongRootXml = "<foo/>";
    const std::vector<uint8_t> wav = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E'};

    struct Refusal {
        std::string name;
        std::vector<uint8_t> bytes;
        const char *why;
    };
    const std::vector<Refusal> refusals = {
        {"sound.wav", wav, "unsupported type .wav"},
        {"truncated.pig", std::vector<uint8_t>(pigBytes.begin(), pigBytes.begin() + pigBytes.size() / 2), "truncated pig"},
        {"rle.tga", rle, "RLE TGA"},
        {"colourmapped.tga", colourMapped, "colour-mapped TGA"},
        {"footer.tga", footer, "TGA with bytes after the pixels"},
        {"notpvr.pvr", pvrNotPvr, ".pvr without PVR3 magic"},
        {"broken.xml", std::vector<uint8_t>(badXml.begin(), badXml.end()), "XML that does not parse"},
        {"other.xml", std::vector<uint8_t>(wrongRootXml.begin(), wrongRootXml.end()), "XML with the wrong root"},
        {"shader.fs", std::vector<uint8_t>{'v', 'o', 'i', 'd'}, "GLSL source (not imported in M2)"},
    };
    for (const Refusal &r : refusals) {
        const std::string src = work + "/inputs/rejected/" + r.name;
        Check(Write(src, r.bytes), "write refusal input " + r.name);
        std::string err;
        bool ok = project::ImportAsset(p, src, err);
        Check(!ok && !err.empty(), std::string("refused: ") + r.why + (err.empty() ? "" : " (" + err + ")"));
    }
    {
        std::string err;
        bool ok = project::ImportAsset(p, pigs[0], err);
        Check(!ok, "a second import of the same pig is refused (" + err + ")");
    }
    Check(p.assets.size() == imported, "refusals leave the database unchanged");

    // Every file under the project's folders is a database entry (no stray copies from the refusals)
    size_t files = 0;
    for (const char *folder : project::kLayoutFolders)
        for (const fs::directory_entry &e : fs::recursive_directory_iterator(fs::path(root) / folder, ec))
            if (e.is_regular_file(ec) && e.path().filename() != "database.txt") files++;
    Check(files == imported, "project folders hold exactly the database entries");

    // Reopen
    project::Project r;
    const bool reopened = project::OpenProject(root, r, error);
    Check(reopened, "reopen project" + (reopened ? std::string() : ": " + error));
    Check(r.manifest.name == "ImportTest" && r.manifest.format == 1, "manifest name and format after reopen");
    Check(r.assets.size() == imported, "reopened database has six entries");

    struct Expect {
        std::string path, type, source;
    };
    const std::vector<Expect> expect = {
        {"models/" + fs::path(pigs[0]).filename().string(), "pig", pigs[0]},
        {"models/" + fs::path(pigs[1]).filename().string(), "pig", pigs[1]},
        {"models/" + fs::path(pigs[2]).filename().string(), "pig", pigs[2]},
        {"textures/made_32.tga", "tga", tgaSource},
        {"shaders/" + fs::path(shaders[0]).filename().string(), "shader", shaders[0]},
        {"textures/" + fs::path(pvrs[0]).filename().string(), "pvr3", pvrs[0]},
    };
    for (const Expect &e : expect) {
        const project::AssetRecord *a = project::FindAsset(r, e.path);
        Check(a != nullptr, "database entry " + e.path);
        if (!a) continue;
        Check(a->type == e.type && a->source == e.source, "type and source of " + e.path);
        std::vector<uint8_t> bytes;
        const bool readOk = Read((fs::path(root) / e.path).string(), bytes);
        Check(readOk && a->size == bytes.size(), "size of " + e.path + " matches its file");
        Check(readOk && a->hash == project::ContentHash(bytes.data(), bytes.size()) && a->hash.size() == 71,
              "hash of " + e.path + " matches its file");
        std::vector<uint8_t> source;
        Read(e.source, source);
        Check(readOk && bytes == source, "copy of " + e.path + " is byte-identical to its source");
    }

    // The copies read back as what they are
    {
        std::vector<uint8_t> bytes;
        Read((fs::path(root) / "models" / fs::path(pigs[0]).filename()).string(), bytes);
        pig::Model m;
        std::string err;
        std::vector<uint8_t> written;
        Check(pig::ReadPig(bytes, m, err) && pig::WritePig(m, written, err) && written == bytes,
              "imported pig reads and writes back byte-identical");
    }
    {
        std::vector<uint8_t> bytes;
        Read((fs::path(root) / "textures" / "made_32.tga").string(), bytes);
        tga::Image img;
        std::string err;
        Check(tga::ReadTga(bytes, img, err) && img.width == 4 && img.height == 3 && img.depth == 32 &&
                  img.id.size() == 3 && img.id[0] == 'A' && img.pixels.size() == 48 && img.pixels[24] == 80 &&
                  img.pixels[25] == 70 && img.pixels[26] == 100 && img.pixels[27] == 253,
              "imported TGA reads back: 4x3, 32-bit, pixel (2,1)");
    }

    // Manifest and database bytes are the canonical text
    {
        std::vector<uint8_t> manifest, db;
        Read((fs::path(root) / "project.jetproj").string(), manifest);
        const std::string dbPath = (fs::path(root) / project::kDatabaseFile).string();
        Read(dbPath, db);
        Check(std::string(manifest.begin(), manifest.end()) == project::FormatManifest(r.manifest),
              "manifest file is the canonical text");
        Check(std::string(db.begin(), db.end()) == project::FormatDatabase(r.assets),
              "database file is the canonical text");
        const bool saved = project::SaveDatabase(r, error);
        std::vector<uint8_t> db2;
        Read(dbPath, db2);
        Check(saved && db2 == db, "saving the reopened database writes the same bytes");
    }

    // Verification passes, then catches a changed byte and a missing file
    std::vector<std::string> problems;
    Check(project::VerifyAssets(r, problems) && problems.empty(), "verify: all assets match");
    const std::string tgaFull = (fs::path(root) / "textures" / "made_32.tga").string();
    std::vector<uint8_t> tgaCopy;
    Read(tgaFull, tgaCopy);
    std::vector<uint8_t> damaged = tgaCopy;
    damaged[damaged.size() - 1] ^= 0x01;
    Write(tgaFull, damaged);
    Check(!project::VerifyAssets(r, problems) && problems.size() == 1 && problems[0] == "hash differs: textures/made_32.tga",
          "verify: a changed byte is reported");
    Write(tgaFull, tgaCopy);
    fs::remove((fs::path(root) / "shaders" / fs::path(shaders[0]).filename()).string(), ec);
    Check(!project::VerifyAssets(r, problems) && problems.size() == 1 &&
              problems[0] == "missing: shaders/" + fs::path(shaders[0]).filename().string(),
          "verify: a missing file is reported");

    std::printf("%d of %d checks failed\n", g_failed, g_checks);
    return g_failed ? 1 : 0;
}
