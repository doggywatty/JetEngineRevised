// core/claraloader/src/recorddb.cpp
#include "recorddb.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include "pugixml.hpp"
#include "encoder.h"
#include "encryption.h"
#include "stream/memlinkstream.h"
#include "stream/redundantstream.h"
#include "stream/streamutil.h"
#include "system.h"

namespace clara {

unsigned Record::GetSize() const
{
    switch (m_type) {
    case TypeDef::BOOL:
    case TypeDef::S32:
    case TypeDef::FLOAT: return 4;
    case TypeDef::S64:
    case TypeDef::DOUBLE: return 8;
    case TypeDef::STRING: return unsigned(m_string.size()) + 2;
    case TypeDef::BINARY: return unsigned(m_binary.size());
    case TypeDef::RECORD_DB: {
        unsigned n = 4;
        if (m_db)
            m_db->ForEach([&](const jet::String&, const Record& r) { n += r.GetSize(); });
        return n;
    }
    default: return 0;
    }
}

void Record::Set(jet::stream::IStream& s)
{
    std::vector<u8> v(s.Size() - s.Tell());
    if (!v.empty())
        s.Read(v.data(), unsigned(v.size()));
    Set(v);
}

bool Record::operator==(const Record& o) const
{
    if (m_type != o.m_type)
        return false;
    switch (m_type) {
    case TypeDef::BOOL:
    case TypeDef::S32: return m_i32 == o.m_i32;
    case TypeDef::S64: return m_i64 == o.m_i64;
    case TypeDef::FLOAT: return m_f32 == o.m_f32;
    case TypeDef::DOUBLE: return m_f64 == o.m_f64;
    case TypeDef::STRING: return m_string == o.m_string;
    case TypeDef::BINARY: return m_binary == o.m_binary;
    case TypeDef::RECORD_DB: return m_db && o.m_db ? *m_db == *o.m_db : m_db == o.m_db;
    default: return true;
    }
}

// u8 type, u32 size, payload.
void Record::SaveBinary(jet::stream::IStreamW& s) const
{
    u8 t = u8(m_type.underlying());
    s << t << GetSize();
    switch (m_type) {
    case TypeDef::BOOL:
    case TypeDef::S32: s << m_i32; break;
    case TypeDef::FLOAT: s.Write(&m_f32, 4); break;
    case TypeDef::S64:
    case TypeDef::DOUBLE: s.Write(&m_i64, 8); break;
    case TypeDef::STRING: jet::WriteString(s, m_string); break;
    case TypeDef::BINARY:
        if (!m_binary.empty())
            s.Write(m_binary.data(), unsigned(m_binary.size()));
        break;
    case TypeDef::RECORD_DB:
        if (m_db)
            m_db->SaveBinary(s);
        break;
    default: break;
    }
}

bool Record::LoadBinary(jet::stream::IStream& s)
{
    u8 t = 0;
    u32 size = 0;
    s >> t >> size;
    m_type = Type(int(t));
    switch (m_type) {
    case TypeDef::NONE: s.SeekCur(int(size)); return true;
    case TypeDef::BOOL:
    case TypeDef::S32: s >> m_i32; break;
    case TypeDef::S64: s.Read(&m_i64, 8); break;
    case TypeDef::FLOAT: s.Read4(&m_f32); break;
    case TypeDef::DOUBLE: s >> m_f64; break;
    case TypeDef::STRING: s >> m_string; break;
    case TypeDef::BINARY:
        if (!size) {
            m_binary.clear();
            return true;
        }
        if (s.Size() - s.Tell() < size)
            return false;
        m_binary.resize(size);
        s.Read(m_binary.data(), size);
        break;
    case TypeDef::RECORD_DB:
        if (!m_db)
            m_db = std::make_shared<RecordDB>();
        m_db->SetFormat(RecordDB::FormatDef::BINARY);
        return m_db->LoadBinary(s);
    default: break;
    }
    return true;
}

bool Record::LoadXML(pugi::xml_node& n)
{
    m_type = Type(int(n.attribute("type").as_uint()));
    if (m_type == TypeDef::BINARY) {
        const char* v = n.child_value();
        size_t len = strlen(v);
        std::vector<u8> out;
        if (len)
            jet::encoder::DecodeBase64(unsigned(len), reinterpret_cast<const u8*>(v), out);
        m_binary = out;
        return true;
    }
    if (m_type == TypeDef::RECORD_DB) {
        if (!m_db)
            m_db = std::make_shared<RecordDB>();
        m_db->SetFormat(RecordDB::FormatDef::XML);
        return m_db->LoadXML(n);
    }
    pugi::xml_attribute a = n.attribute("value");
    switch (m_type) {
    case TypeDef::BOOL: m_i32 = a.as_bool(); break;
    case TypeDef::S64: sscanf(a.value(), "%lld", &m_i64); break;
    case TypeDef::S32: m_i32 = a.as_int(); break;
    case TypeDef::FLOAT: m_f32 = a.as_float(); break;
    case TypeDef::DOUBLE: m_f64 = a.as_double(); break;
    case TypeDef::STRING: m_string = a.value(); break;
    default: break;
    }
    return true;
}

void Record::SaveXML(pugi::xml_node& n) const
{
    n.append_attribute("type").set_value(int(m_type.underlying()));
    if (m_type == TypeDef::BINARY) {
        pugi::xml_node d = n.append_child(pugi::node_pcdata);
        if (!m_binary.empty()) {
            std::vector<u8> out;
            jet::encoder::EncodeBase64(unsigned(m_binary.size()), m_binary.data(), out);
            out.push_back(0);
            d.set_value(reinterpret_cast<const char*>(out.data()));
        }
        return;
    }
    if (m_type == TypeDef::RECORD_DB) {
        if (m_db)
            m_db->SaveXML(n);
        return;
    }
    pugi::xml_attribute a = n.append_attribute("value");
    char buf[64];
    switch (m_type) {
    case TypeDef::BOOL:
    case TypeDef::S32: a.set_value(m_i32); break;
    case TypeDef::S64:
        snprintf(buf, sizeof(buf), "%lld", m_i64);
        a.set_value(buf);
        break;
    case TypeDef::FLOAT: a.set_value(double(m_f32)); break;
    case TypeDef::DOUBLE: a.set_value(m_f64); break;
    case TypeDef::STRING: a.set_value(m_string.c_str()); break;
    default: break;
    }
}

RecordDB::RecordDB()
    : m_file(jet::String::null, jet::stream::AtomicFileStream::OpenModeDef::READ), m_format(FormatDef::AUTO), m_encryption(EncryptionDef::NONE),
      m_redundancy(0)
{
    m_null.SetReadOnly();
    SetPath(jet::String::null);
}

RecordDB::RecordDB(const jet::String& name) : RecordDB() { m_name = name; }

void RecordDB::SetFormat(Format f)
{
    if (m_redundancy && f != FormatDef::BINARY) {
        printf("RecordDB: redundancy needs the binary format\n");
        f = FormatDef::BINARY;
    }
    m_format = f;
}

void RecordDB::SetRedundancyCount(unsigned n)
{
    m_redundancy = n;
    if (n)
        m_format = FormatDef::BINARY;
}

// Ensures a single trailing '/'. An empty path means the user folder.
void RecordDB::SetPath(const jet::String& path)
{
    std::string p = path.empty() ? jet::System::GetUserFolder().c_str() : path.c_str();
    if (!p.empty() && p.back() != '/')
        p += '/';
    while (p.size() > 1 && p[p.size() - 1] == '/' && p[p.size() - 2] == '/')
        p.pop_back();
    m_path = jet::String(p);
}

unsigned RecordDB::GetSize() const
{
    unsigned n = 4;
    for (const auto& kv : m_records)
        n += kv.second.GetSize();
    return n;
}

Record& RecordDB::Get(const Path& p)
{
    if (p.IsEmpty())
        return m_null;
    if (p.GetCount() == 1) {
        auto it = m_records.find(p.Get(0));
        return it == m_records.end() ? m_null : it->second;
    }
    Record& r = Get(p.GetSubPath(0, 1));
    if (r.GetType() != Record::TypeDef::RECORD_DB || !r.GetAsRecordDB())
        return m_null;
    return r.GetAsRecordDB()->Get(p.GetSubPath(1, 0));
}

bool RecordDB::Has(const Path& p) const
{
    if (p.IsEmpty())
        return false;
    if (p.GetCount() == 1)
        return m_records.count(p.Get(0)) != 0;
    const Record& r = Get(p.GetSubPath(0, 1));
    if (r.GetType() != Record::TypeDef::RECORD_DB || !r.GetAsRecordDB())
        return false;
    return r.GetAsRecordDB()->Has(p.GetSubPath(1, 0));
}

// A key that exists with a different type is left untouched (the null record is returned).
Record& RecordDB::Set(const Path& p, const Record& r)
{
    if (p.IsEmpty())
        return m_null;
    if (p.GetCount() == 1) {
        const jet::String& k = p.Get(0);
        if (k.empty())
            return m_null;
        auto it = m_records.find(k);
        if (it == m_records.end())
            return m_records.emplace(k, r).first->second;
        if (it->second.GetType() != r.GetType())
            return m_null;
        it->second = r;
        return it->second;
    }
    Record sub(Record::TypeDef::RECORD_DB);
    sub.m_db = std::make_shared<RecordDB>();
    Record& d = GetOrAdd(p.GetSubPath(0, 1), sub);
    if (d.GetType() != Record::TypeDef::RECORD_DB || !d.GetAsRecordDB())
        return m_null;
    return d.GetAsRecordDB()->Set(p.GetSubPath(1, 0), r);
}

Record& RecordDB::GetOrAdd(const Path& p, const Record& r)
{
    if (p.IsEmpty())
        return m_null;
    if (p.GetCount() == 1)
        return Has(p) ? Get(p) : Set(p, r);
    Path head = p.GetSubPath(0, 1);
    if (!Has(head))
        return Set(p, r);
    Record& d = Get(head);
    if (d.GetType() != Record::TypeDef::RECORD_DB || !d.GetAsRecordDB())
        return m_null;
    return d.GetAsRecordDB()->GetOrAdd(p.GetSubPath(1, 0), r);
}

bool RecordDB::Remove(const Path& p)
{
    if (p.IsEmpty())
        return false;
    if (p.GetCount() == 1)
        return m_records.erase(p.Get(0)) != 0;
    Record& d = Get(p.GetSubPath(0, 1));
    if (d.GetType() != Record::TypeDef::RECORD_DB || !d.GetAsRecordDB())
        return false;
    return d.GetAsRecordDB()->Remove(p.GetSubPath(1, 0));
}

bool RecordDB::operator==(const RecordDB& o) const
{
    if (m_records.size() != o.m_records.size())
        return false;
    for (const auto& kv : m_records) {
        auto it = o.m_records.find(kv.first);
        if (it == o.m_records.end() || it->second != kv.second)
            return false;
    }
    return true;
}

// Binary files start with 0xED, u32 encryption; plain: u32 crc of the rest, raw data;
// XTEA: u32 encrypted size, u32 plain size, XTEA([u32 crc][raw data]). Anything else is XML.
bool RecordDB::Load(jet::stream::IStream& src)
{
    m_records.clear();
    std::unique_ptr<jet::stream::RedundantStream> red;
    jet::stream::IStream* s = &src;
    if (m_redundancy) {
        red.reset(new jet::stream::RedundantStream);
        red->SetSource(src, false);
        s = red.get();
    }
    s->Start();
    bool ok = false;
    if (s->IsStarted() && s->Size()) {
        u8 magic = 0;
        *s >> magic;
        if (magic == 0xed) {
            if (m_format != FormatDef::XML) {
                SetFormat(FormatDef::BINARY);
                u32 enc = 0;
                *s >> enc;
                m_encryption = Encryption(int(enc));
                if (enc == EncryptionDef::NONE) {
                    u32 crc = 0;
                    *s >> crc;
                    unsigned pos = s->Tell();
                    if (jet::stream::ComputeCRC(*s, s->Size() - pos) == crc) {
                        s->SeekBeg(pos);
                        ok = LoadRaw(*s);
                    }
                } else {
                    u32 encSize = 0, plain = 0;
                    *s >> encSize;
                    if (encSize <= s->Size() - s->Tell() && encSize <= 0x2000000) {
                        *s >> plain;
                        if (plain <= 0x2000000) {
                            unsigned padded = (plain + 8) & ~7u;
                            std::vector<u32> buf(padded / 4);
                            s->Read(buf.data(), padded);
                            jet::encryption::DecipherXTEA(buf.data(), unsigned(buf.size()), m_key);
                            jet::stream::MemLinkStream m(reinterpret_cast<u8*>(buf.data()), plain);
                            u32 crc = 0;
                            m >> crc;
                            unsigned pos = m.Tell();
                            if (jet::stream::ComputeCRC(m, m.Size() - pos) == crc) {
                                m.SeekBeg(pos);
                                ok = LoadRaw(m);
                            }
                        }
                    }
                }
            }
        } else if (m_format != FormatDef::BINARY) {
            SetFormat(FormatDef::XML);
            s->SeekCur(-1);
            ok = LoadRaw(*s);
        }
    }
    s->Finish();
    return ok;
}

bool RecordDB::LoadRaw(jet::stream::IStream& s)
{
    if (m_format == FormatDef::BINARY)
        return LoadBinary(s);
    if (m_format != FormatDef::XML)
        return false;
    unsigned n = s.Size() - s.Tell();
    std::vector<char> buf(n + 1, 0);
    s.Read(buf.data(), n);
    pugi::xml_document doc;
    if (!doc.load_buffer(buf.data(), n, pugi::parse_default))
        return false;
    pugi::xml_node root = doc.child("root");
    return root && LoadXML(root);
}

// u32 count, count x (key, record).
bool RecordDB::LoadBinary(jet::stream::IStream& s)
{
    m_format = FormatDef::BINARY;
    u32 n = 0;
    s >> n;
    for (u32 i = 0; i < n; ++i) {
        jet::String k;
        s >> k;
        Record r;
        if (!r.LoadBinary(s))
            return false;
        Set(Path(k), r);
    }
    return true;
}

bool RecordDB::LoadXML(pugi::xml_node& n)
{
    for (pugi::xml_node c = n.child("record"); c; c = c.next_sibling("record")) {
        Record r;
        if (!r.LoadXML(c))
            return false;
        Set(Path(c.attribute("id").value()), r);
    }
    return true;
}

void RecordDB::SaveXML(pugi::xml_node& n) const
{
    for (const auto& kv : m_records) {
        pugi::xml_node c = n.append_child("record");
        c.append_attribute("id").set_value(kv.first.c_str());
        if (kv.second.GetType() == Record::TypeDef::RECORD_DB && kv.second.GetAsRecordDB())
            kv.second.GetAsRecordDB()->m_format = FormatDef::XML;
        kv.second.SaveXML(c);
    }
}

void RecordDB::SaveBinary(jet::stream::IStreamW& s) const
{
    s << GetCount();
    for (const auto& kv : m_records) {
        jet::WriteString(s, kv.first);
        if (kv.second.GetType() == Record::TypeDef::RECORD_DB && kv.second.GetAsRecordDB())
            kv.second.GetAsRecordDB()->m_format = FormatDef::BINARY;
        kv.second.SaveBinary(s);
    }
}

struct XmlWriter : pugi::xml_writer {
    std::string out;
    void write(const void* data, size_t size) override { out.append(static_cast<const char*>(data), size); }
};

void RecordDB::SaveRaw(jet::stream::IStreamW& s) const
{
    if (m_format == FormatDef::XML) {
        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child("root");
        SaveXML(root);
        XmlWriter w;
        doc.save(w, "\t", pugi::format_default, pugi::encoding_auto);
        s.Write(w.out.data(), unsigned(w.out.size()));
    } else if (m_format == FormatDef::BINARY) {
        SaveBinary(s);
    }
}

void RecordDB::Save(jet::stream::IStreamW& dst) const
{
    std::unique_ptr<jet::stream::RedundantStream> red;
    jet::stream::IStreamW* s = &dst;
    if (m_redundancy) {
        red.reset(new jet::stream::RedundantStream);
        red->SetDestination(dst, m_redundancy);
        s = red.get();
    }
    s->Start();
    if (m_format == FormatDef::AUTO)
        m_format = FormatDef::BINARY;
    if (m_format == FormatDef::XML) {
        SaveRaw(*s);
    } else if (m_format == FormatDef::BINARY) {
        u8 magic = 0xed;
        u32 enc = u32(m_encryption.underlying());
        *s << magic << enc;
        jet::stream::MemoryStream body;
        body.Start();
        u32 zero = 0;
        body << zero;
        SaveRaw(body);
        std::vector<u8>& b = body.GetInternalArray();
        u32 crc = jet::stream::ComputeCRC(b.data() + 4, unsigned(b.size()) - 4);
        memcpy(b.data(), &crc, 4);
        if (m_encryption == EncryptionDef::NONE) {
            s->Write(b.data(), unsigned(b.size()));
        } else {
            std::vector<u32> enc32;
            jet::encryption::EncipherXTEA(b.data(), unsigned(b.size()), m_key, enc32);
            jet::stream::MemoryStream out;
            u32 plain = unsigned(b.size());
            out << plain;
            out.Write(enc32.data(), unsigned(enc32.size() * 4));
            u32 encSize = out.Size();
            *s << encSize;
            s->Write(out.GetInternalArray().data(), encSize);
        }
        body.Finish();
    }
    s->Finish();
}

bool RecordDB::Load()
{
    m_records.clear();
    m_file.Open(GetFilename(), jet::stream::AtomicFileStream::OpenModeDef::READ);
    if (!m_file.IsOpen())
        return false;
    bool ok = Load(m_file);
    if (m_file.IsOpen())
        m_file.Close();
    return ok;
}

void RecordDB::Save() const
{
    jet::String f = GetFilename();
    m_file.Open(f, jet::stream::AtomicFileStream::OpenModeDef::WRITE);
    if (!m_file.IsOpen()) {
        printf("RecordDB: cannot open '%s' for writing\n", f.c_str());
        return;
    }
    Save(m_file);
    if (m_file.IsOpen())
        m_file.Close();
}

void RecordDB::Delete()
{
    jet::String f = GetFilename();
    if (!f.empty())
        jet::stream::DeleteFile(f);
    Clear();
}

bool RecordDB::Exists() const { return jet::stream::IsFile(GetFilename()); }

}
