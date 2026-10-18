// core/jet/src/stream/streamutil.cpp
#include "stream/streamutil.h"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include "stream/dirstreamfactory.h"
#include "stream/filesystemdirstreamfactory.h"
#include "stream/ifilesystem.h"
#include "stream/istream.h"
#include "stream/libstreamfactory.h"
#include "stream/zipstreamfactory.h"

namespace fs = std::filesystem;

namespace jet {
namespace stream {

static fs::path P(const String& s) { return fs::u8path(s.c_str()); }

String GetFixedCasePath(const String& path)
{
    unsigned i = path.find_last_of('/');
    if (i == String::npos)
        return path.GetLowercase();
    return path.substr(0, i) + path.substr(i).GetLowercase();
}

std::shared_ptr<IStreamFactory> CreateStreamFactoryFromPath(const String& path, const String& ext)
{
    return CreateStreamFactoryFromPath(nullptr, path, ext);
}

std::shared_ptr<IStreamFactory> CreateStreamFactoryFromPath(std::shared_ptr<IFileSystem> fsys, const String& path, const String& ext)
{
    std::shared_ptr<IStreamFactory> f;
    if (path.empty())
        return f;
    if (!fsys) {
        if (IsDirectory(path))
            f = std::make_shared<DirStreamFactory>(path, ext);
        else if (IsFile(path)) {
            if (path.find(".zip") != String::npos || path.find(".jpk") != String::npos)
                f = std::make_shared<ZipStreamFactory>(path, ext);
            else
                f = std::make_shared<LibStreamFactory>(path, ext);
        }
    } else {
        const IFileSystem::Entry& e = fsys->FindEntryByPath(0, path);
        if (e.id >= 0) {
            if (e.isDirectory)
                f = std::make_shared<FileSystemDirStreamFactory>(fsys, path, ext);
            else if (path.find(".zip") != String::npos) {
                printf("Zips inside filesystems are not supported: '%s'\n", path.c_str());
                return f;
            } else
                f = std::make_shared<LibStreamFactory>(fsys, path, ext);
        }
    }
    if (!f)
        printf("Cannot find host/file/directory '%s'\n", path.c_str());
    return f;
}

static const u32* CrcTable()
{
    static u32 t[256];
    static bool init = false;
    if (!init) {
        for (u32 i = 0; i < 256; ++i) {
            u32 c = i;
            for (int k = 0; k < 8; ++k)
                c = c & 1 ? 0xedb88320 ^ (c >> 1) : c >> 1;
            t[i] = c;
        }
        init = true;
    }
    return t;
}

u32 ComputeCRC(const u8* data, unsigned size)
{
    const u32* t = CrcTable();
    u32 c = 0xffffffff;
    for (unsigned i = 0; i < size; ++i)
        c = t[(data[i] ^ c) & 0xff] ^ (c >> 8);
    return ~c;
}

u32 ComputeCRC(IStream& s, unsigned size)
{
    const u32* t = CrcTable();
    u32 c = 0xffffffff;
    unsigned pos = s.Tell();
    for (; size; --size) {
        u8 b = 0;
        s >> b;
        c = t[(b ^ c) & 0xff] ^ (c >> 8);
    }
    s.SeekBeg(pos);
    return ~c;
}

// The filter is accepted but, as in the shipped code, not applied.
std::vector<String> EnumerateFiles(const String& dir, const String&)
{
    std::vector<String> r;
    std::error_code ec;
    for (fs::directory_iterator it(P(dir), ec), end; !ec && it != end; it.increment(ec))
        if (!it->is_directory(ec))
            r.push_back(String(it->path().filename().u8string()));
    return r;
}

std::vector<String> EnumerateFolders(const String& dir)
{
    std::vector<String> r;
    std::error_code ec;
    for (fs::directory_iterator it(P(dir), ec), end; !ec && it != end; it.increment(ec))
        if (it->is_directory(ec))
            r.push_back(String(it->path().filename().u8string()));
    return r;
}

static fs::path Trimmed(const String& path)
{
    std::string s = path.c_str();
    if (!s.empty() && (s.back() == '/' || s.back() == '\\'))
        s.pop_back();
    return fs::u8path(s);
}

bool IsFile(const String& path)
{
    std::error_code ec;
    return !path.empty() && fs::exists(Trimmed(path), ec) && !fs::is_directory(Trimmed(path), ec);
}

bool IsDirectory(const String& path)
{
    std::error_code ec;
    return !path.empty() && fs::is_directory(Trimmed(path), ec);
}

bool GetFileTime(const String& path, long& time)
{
    std::error_code ec;
    auto t = fs::last_write_time(P(path), ec);
    if (ec)
        return false;
    time = long(std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count());
    return true;
}

bool SetFileTime(const String& path, const long& time)
{
    std::error_code ec;
    fs::last_write_time(P(path), fs::file_time_type(std::chrono::seconds(time)), ec);
    return !ec;
}

bool MakeDirectory(const String& path)
{
    String p = GetNormalizedPath(path + '/');
    int made = 0;
    for (unsigned i = 0; (i = p.find_first_of('/', i)) != String::npos;) {
        String sub = p.substr(0, i);
        ++i;
        std::error_code ec;
        if (!sub.empty() && !IsDirectory(sub) && fs::create_directory(P(sub), ec))
            ++made;
    }
    return made != 0;
}

bool DeleteFile(const String& path)
{
    std::error_code ec;
    return fs::remove(P(path), ec);
}

// Deletes the files in the directory (not subdirectories), as the original does.
bool DeleteDirectory(const String& path)
{
    String dir = GetNormalizedPath(path + '/');
    for (const String& f : EnumerateFiles(dir, "*"))
        DeleteFile(dir + f);
    return false;
}

int ReplaceBackslash(int c) { return c == '\\' ? '/' : c; }

String GetNormalizedPath(const String& path)
{
    std::string s = path.c_str();
    for (char& c : s)
        c = char(ReplaceBackslash(c));
    for (size_t i; (i = s.find("//")) != std::string::npos;)
        s.erase(i, 1);
    return String(s);
}

bool Rename(const String& from, const String& to)
{
    return !from.empty() && !to.empty() && rename(from.c_str(), to.c_str()) == 0;
}

}
}
