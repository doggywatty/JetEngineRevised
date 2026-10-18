// core/jet/src/stream/zipfilesystem.cpp
#include "stream/zipfilesystem.h"
#include <cstdio>
#include <map>
#include <zlib.h>
#include "stream/filestream.h"
#include "stream/memorystream.h"
#include "stream/mmapstream.h"

namespace jet {
namespace stream {

ZipFileSystem::ZipFileSystem(const String& path, u32 signature) : m_path(path), m_signature(signature), m_deobfuscator(nullptr)
{
    FileStream f(path, FileStream::k_in, true);
    LoadFileSystem(&f);
}

ZipFileSystem::ZipFileSystem(const String& path, Deobfuscator* d) : m_path(path), m_signature(0x04034b50), m_deobfuscator(d)
{
    FileStream f(path, FileStream::k_in, true);
    LoadFileSystem(&f);
}

ZipFileSystem::~ZipFileSystem() { delete m_deobfuscator; }

const IFileSystem::Entry& ZipFileSystem::GetEntry(unsigned id) const
{
    return id < m_entries.size() ? m_entries[id].entry : m_emptyEntry;
}

const IFileSystem::Entry& ZipFileSystem::GetEntry(unsigned parent, unsigned child) const
{
    if (parent < m_entries.size() && child < m_entries[parent].entry.childrenCount)
        return m_entries[m_entries[parent].children[child]].entry;
    return m_emptyEntry;
}

const IFileSystem::Entry& ZipFileSystem::FindEntryByName(unsigned from, const String& name, bool recursive) const
{
    if (from >= m_entries.size())
        return m_emptyEntry;
    std::vector<const EntryData*> queue(1, &m_entries[from]);
    for (size_t q = 0; q < queue.size(); ++q) {
        for (unsigned c : queue[q]->children) {
            const EntryData& e = m_entries[c];
            if (e.entry.name.Equals(name))
                return e.entry;
            if (recursive && !e.children.empty())
                queue.push_back(&e);
        }
    }
    return m_emptyEntry;
}

const IFileSystem::Entry& ZipFileSystem::FindEntryByPath(unsigned from, const String& path) const
{
    if (path.Equals("/") || path.empty())
        return GetEntry(from);
    if (from >= m_entries.size())
        return m_emptyEntry;
    std::vector<String> parts;
    for (unsigned b = 0, e; b <= path.size(); b = e + 1) {
        e = path.find_first_of('/', b);
        if (e == String::npos)
            e = path.size();
        if (e > b)
            parts.push_back(path.substr(b, e - b));
    }
    const EntryData* cur = &m_entries[from];
    for (size_t i = 0; i < parts.size(); ++i) {
        const EntryData* next = nullptr;
        for (unsigned c : cur->children)
            if (m_entries[c].entry.name.Equals(parts[i]))
                next = &m_entries[c];
        if (!next)
            return m_emptyEntry;
        if (i + 1 == parts.size())
            return next->entry;
        cur = next;
    }
    return m_emptyEntry;
}

String ZipFileSystem::GetEntryFullPath(unsigned id) const
{
    if (id >= m_entries.size())
        return String::null;
    std::vector<String> parts;
    for (; id; id = m_entries[id].entry.parentId)
        parts.push_back(m_entries[id].entry.name);
    std::string r;
    for (size_t i = parts.size(); i--;) {
        r += parts[i].c_str();
        if (i)
            r += '/';
    }
    return String(r);
}

bool ZipFileSystem::CanCreateStream(unsigned id) const { return id < m_entries.size() && !m_entries[id].entry.isDirectory; }

IStream* ZipFileSystem::CreateStream(unsigned id)
{
    if (!CanCreateStream(id))
        return nullptr;
    const EntryData& e = m_entries[id];
    const ZipFileEntry& z = e.zipData;
    if (z.method == 0) {
        if (!z.size)
            return new MemoryStream;
        return new MMapStream(m_path, z.dataOffset, z.size);
    }
    if (z.method != 8) {
        printf("'%s' in '%s': unsupported compression method %d\n", e.entry.name.c_str(), m_path.c_str(), z.method);
        return nullptr;
    }
    MemoryStream* out = new MemoryStream;
    std::vector<u8>& buf = out->GetInternalArray();
    buf.resize(z.size);
    if (!z.size)
        return out;
    MMapStream src(m_path, z.dataOffset, z.compressedSize);
    src.Start();
    const u8* in = static_cast<const u8*>(src.Map(z.compressedSize));
    z_stream zs = {};
    zs.next_in = const_cast<Bytef*>(in);
    zs.avail_in = z.compressedSize;
    zs.next_out = buf.data();
    zs.avail_out = z.size;
    int r = in ? inflateInit2(&zs, -MAX_WBITS) : Z_STREAM_ERROR;
    if (r == Z_OK) {
        r = inflate(&zs, Z_FINISH);
        inflateEnd(&zs);
    }
    src.Unmap();
    src.Finish();
    if (r != Z_STREAM_END) {
        printf("Failed to inflate '%s' in '%s'\n", e.entry.name.c_str(), m_path.c_str());
        delete out;
        return nullptr;
    }
    out->SeekBeg(0);
    return out;
}

void ZipFileSystem::LoadFileSystem(IStream* s)
{
    m_entries.clear();
    EntryData root;
    root.entry.id = 0;
    root.entry.isDirectory = true;
    m_entries.push_back(root);
    if (!s)
        return;
    s->Start();
    if (!s->IsStarted())
        return;
    std::map<String, unsigned> dirs;
    std::string name;
    auto getDir = [&](const String& path) -> unsigned {
        unsigned parent = 0;
        std::string acc;
        for (unsigned b = 0, e; b < path.size(); b = e + 1) {
            e = path.find_first_of('/', b);
            if (e == String::npos)
                e = path.size();
            String part = path.substr(b, e - b);
            acc += acc.empty() ? part.c_str() : (std::string("/") + part.c_str());
            auto it = dirs.find(String(acc));
            if (it != dirs.end()) {
                parent = it->second;
                continue;
            }
            EntryData d;
            d.entry.id = int(m_entries.size());
            d.entry.isDirectory = true;
            d.entry.name = part;
            d.entry.parentId = parent;
            m_entries.push_back(d);
            m_entries[parent].children.push_back(unsigned(d.entry.id));
            ++m_entries[parent].entry.childrenCount;
            dirs[String(acc)] = parent = unsigned(d.entry.id);
        }
        return parent;
    };
    while (s->Tell() < s->Size()) {
        LocalHeader h;
        *s >> h.signature >> h.version >> h.flags >> h.method >> h.time >> h.date >> h.crc >> h.compressedSize >> h.size >> h.nameLength >>
            h.extraLength;
        if (m_deobfuscator && !m_deobfuscator->Deobfuscate(h))
            break;
        if (h.signature != m_signature)
            break;
        unsigned left = s->Size() - s->Tell();
        if (h.compressedSize >= left || h.nameLength > 0x2000 || h.nameLength > left) {
            printf("Corrupted zip '%s'\n", m_path.c_str());
            break;
        }
        name.assign(h.nameLength, '\0');
        s->Read(&name[0], h.nameLength);
        bool isDir = !name.empty() && (name.back() == '/' || name.back() == '\\');
        if (isDir)
            name.pop_back();
        if (h.extraLength)
            s->SeekCur(h.extraLength);
        if (h.flags & 8)
            *s >> h.crc >> h.compressedSize >> h.size;
        u32 dataOffset = s->Tell();
        s->SeekCur(int(h.compressedSize));
        size_t slash = name.find_last_of('/');
        String dirPath = slash == std::string::npos ? String::null : String(name.substr(0, slash));
        String leaf(slash == std::string::npos ? name : name.substr(slash + 1));
        unsigned parent = dirPath.empty() ? 0 : getDir(dirPath);
        if (isDir) {
            getDir(String(name));
            continue;
        }
        EntryData d;
        d.entry.id = int(m_entries.size());
        d.entry.name = leaf;
        d.entry.parentId = parent;
        d.zipData.dataOffset = dataOffset;
        d.zipData.method = h.method;
        d.zipData.compressedSize = h.compressedSize;
        d.zipData.size = h.size;
        m_entries.push_back(d);
        m_entries[parent].children.push_back(unsigned(d.entry.id));
        ++m_entries[parent].entry.childrenCount;
    }
    s->Finish();
}

}
}
