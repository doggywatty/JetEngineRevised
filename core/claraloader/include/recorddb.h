// core/claraloader/include/recorddb.h
#ifndef CORE_CLARALOADER_INCLUDE_RECORDDB_H
#define CORE_CLARALOADER_INCLUDE_RECORDDB_H

#include <memory>
#include <string>
#include <unordered_map>
#include "clara.h"
#include "jenum.h"
#include "stream/atomicfilestream.h"

namespace pugi {
class xml_node;
}

namespace clara {

class RecordDB;

// A typed value. Read-only records (the null record returned for misses) ignore writes.
class Record {
public:
    struct TypeDef {
        enum type { NONE, BOOL, S64, S32, FLOAT, DOUBLE, STRING, BINARY, RECORD_DB };
    };
    typedef safe_enum<TypeDef> Type;

    Record(Type t = TypeDef::NONE) : m_readOnly(false), m_type(t), m_i64(0) {}
    Record(TypeDef::type t) : Record(Type(t)) {}
    Record(bool v) : Record() { Set(v); }
    Record(int v) : Record() { Set(v); }
    Record(unsigned v) : Record() { Set(v); }
    Record(long long v) : Record() { Set(v); }
    Record(float v) : Record() { Set(v); }
    Record(double v) : Record() { Set(v); }
    Record(const std::string& v) : Record() { Set(v); }
    Record(const jet::String& v) : Record() { Set(v); }
    Record(const u8* p, unsigned n) : Record() { Set(p, n); }
    Record(const Record& o) = default;
    Record& operator=(const Record& o) = default;

    Type GetType() const { return m_type; }
    void SetType(Type t) { if (!m_readOnly) m_type = t; }
    void SetReadOnly() { m_readOnly = true; }
    bool IsReadOnly() const { return m_readOnly; }
    unsigned GetSize() const;

    bool GetAsBool(bool d = false) const { return m_type == TypeDef::BOOL ? m_i32 != 0 : d; }
    int GetAsS32(int d = 0) const { return m_type == TypeDef::S32 ? m_i32 : d; }
    unsigned GetAsU32(unsigned d = 0) const { return unsigned(GetAsS32(int(d))); }
    long long GetAsS64(long long d = 0) const { return m_type == TypeDef::S64 ? m_i64 : d; }
    unsigned long long GetAsU64(unsigned long long d = 0) const { return (unsigned long long)GetAsS64((long long)d); }
    float GetAsFloat(float d = 0) const { return m_type == TypeDef::FLOAT ? m_f32 : d; }
    double GetAsDouble(double d = 0) const { return m_type == TypeDef::DOUBLE ? m_f64 : d; }
    std::string GetAsCString(std::string d = std::string()) const { return m_type == TypeDef::STRING ? m_string : d; }
    jet::String GetAsString(jet::String d = jet::String()) const { return m_type == TypeDef::STRING ? jet::String(m_string) : d; }
    const std::vector<u8>& GetAsBinary() const { return m_binary; }
    std::vector<u8>& GetAsBinary() { return m_binary; }
    RecordDB* GetAsRecordDB() const { return m_db.get(); }

    void Set(bool v) { SetValue(TypeDef::BOOL, [&] { m_i32 = v; }); }
    void Set(int v) { SetValue(TypeDef::S32, [&] { m_i32 = v; }); }
    void Set(unsigned v) { Set(int(v)); }
    void Set(long long v) { SetValue(TypeDef::S64, [&] { m_i64 = v; }); }
    void Set(unsigned long long v) { Set((long long)v); }
    void Set(float v) { SetValue(TypeDef::FLOAT, [&] { m_f32 = v; }); }
    void Set(double v) { SetValue(TypeDef::DOUBLE, [&] { m_f64 = v; }); }
    void Set(const std::string& v) { SetValue(TypeDef::STRING, [&] { m_string = v; }); }
    void Set(const jet::String& v) { Set(std::string(v.c_str(), v.size())); }
    void Set(const u8* p, unsigned n) { SetValue(TypeDef::BINARY, [&] { m_binary.assign(p, p + n); }); }
    void Set(const std::vector<u8>& v) { Set(v.data(), unsigned(v.size())); }
    void Set(jet::stream::IStream& s);

    bool operator==(const Record& o) const;
    bool operator!=(const Record& o) const { return !(*this == o); }

    void SaveBinary(jet::stream::IStreamW& s) const;
    bool LoadBinary(jet::stream::IStream& s);
    bool LoadXML(pugi::xml_node& n);
    void SaveXML(pugi::xml_node& n) const;

private:
    friend class RecordDB;
    template <class F>
    void SetValue(Type t, F f)
    {
        if (m_readOnly)
            return;
        m_type = t;
        f();
    }

    bool m_readOnly;
    Type m_type;
    union {
        int m_i32;
        long long m_i64;
        float m_f32;
        double m_f64;
    };
    std::vector<u8> m_binary;
    std::shared_ptr<RecordDB> m_db;
    std::string m_string;
};

// Key -> Record store with "a/b/c" paths into nested RecordDBs. Saved as binary (optionally XTEA
// encrypted, optionally written redundantly) or as XML.
class RecordDB {
public:
    struct FormatDef {
        enum type { AUTO, BINARY, XML };
    };
    typedef safe_enum<FormatDef> Format;
    struct EncryptionDef {
        enum type { NONE, XTEA };
    };
    typedef safe_enum<EncryptionDef> Encryption;

    RecordDB();
    explicit RecordDB(const jet::String& name);
    ~RecordDB() {}
    RecordDB(const RecordDB&) = delete;
    RecordDB& operator=(const RecordDB&) = delete;

    bool Load();
    bool Load(jet::stream::IStream& s);
    void Save() const;
    void Save(jet::stream::IStreamW& s) const;
    void Delete();
    bool Exists() const;
    void Clear() { m_records.clear(); }

    void SetFormat(Format f);
    Format GetFormat() const { return m_format; }
    void SetEncryption(Encryption e) { m_encryption = e; }
    void SetEncryptionKey(const jet::String& k) { m_key = k; }
    const jet::String& GetEncryptionKey() const { return m_key; }
    void SetRedundancyCount(unsigned n);
    unsigned GetRedundancyCount() const { return m_redundancy; }
    void SetPath(const jet::String& path);
    const jet::String& GetPath() const { return m_path; }
    void SetName(const jet::String& n) { m_name = n; }
    const jet::String& GetName() const { return m_name; }
    jet::String GetFilename() const { return m_name.empty() ? m_path : m_path + m_name; }

    unsigned GetCount() const { return unsigned(m_records.size()); }
    unsigned GetSize() const;
    Record& Set(const Path& p, const Record& r);
    Record& Set(const jet::String& k, const Record& r) { return Set(Path(k), r); }
    Record& Get(const Path& p);
    const Record& Get(const Path& p) const { return const_cast<RecordDB*>(this)->Get(p); }
    Record& Get(const jet::String& k) { return Get(Path(k)); }
    const Record& Get(const jet::String& k) const { return Get(Path(k)); }
    Record& GetOrAdd(const Path& p, const Record& r);
    Record& GetOrAdd(const jet::String& k, const Record& r) { return GetOrAdd(Path(k), r); }
    bool Has(const Path& p) const;
    bool Has(const jet::String& k) const { return Has(Path(k)); }
    bool Remove(const Path& p);
    bool Remove(const jet::String& k) { return Remove(Path(k)); }
    Record& Increment(const Path& p) { return Set(p, Record(Get(p).GetAsS32(0) + 1)); }
    Record& Increment(const jet::String& k) { return Increment(Path(k)); }
    Record& Decrement(const Path& p) { return Set(p, Record(Get(p).GetAsS32(0) - 1)); }
    Record& Decrement(const jet::String& k) { return Decrement(Path(k)); }
    Record& Set(const char* k, const Record& r) { return Set(Path(k), r); }
    Record& Get(const char* k) { return Get(Path(k)); }
    const Record& Get(const char* k) const { return Get(Path(k)); }
    Record& GetOrAdd(const char* k, const Record& r) { return GetOrAdd(Path(k), r); }
    bool Has(const char* k) const { return Has(Path(k)); }
    bool Remove(const char* k) { return Remove(Path(k)); }
    Record& Increment(const char* k) { return Increment(Path(k)); }
    Record& Decrement(const char* k) { return Decrement(Path(k)); }

    template <class F>
    void ForEach(F f) const
    {
        for (const auto& kv : m_records)
            f(kv.first, kv.second);
    }

    bool operator==(const RecordDB& o) const;
    bool operator!=(const RecordDB& o) const { return !(*this == o); }

    bool LoadRaw(jet::stream::IStream& s);
    bool LoadBinary(jet::stream::IStream& s);
    bool LoadXML(pugi::xml_node& n);
    void SaveRaw(jet::stream::IStreamW& s) const;
    void SaveBinary(jet::stream::IStreamW& s) const;
    void SaveXML(pugi::xml_node& n) const;

private:
    Record& Null() { return m_null; }

    jet::String m_path;
    jet::String m_name;
    mutable jet::stream::AtomicFileStream m_file;
    std::unordered_map<jet::String, Record> m_records;
    Record m_null;
    mutable Format m_format;
    Encryption m_encryption;
    jet::String m_key;
    unsigned m_redundancy;
};

}

#endif
