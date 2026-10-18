// libs/social_framework/include/social/cache/cachecommon.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHECOMMON_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHECOMMON_H

namespace social::cache {
struct CacheObjectData {  // sizeof 0x2C
    CacheObjectData(const social::cache::CacheObjectData&);
    CacheObjectData(const char*, unsigned, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    CacheObjectData(const char*, unsigned);
    CacheObjectData();
    ~CacheObjectData();
    void Copy(const social::cache::CacheObjectData&);
    void Release();
    social::cache::Error Write(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::Error Read(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static social::cache::Error Erase(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool IsMetadataFilename(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_metadata_file_suffix;
    char* m_buffer;  // 0x0
    unsigned m_size;  // 0x4
    social::cache::CacheObjectData::Metadata m_metadata;  // 0x8
private:
    bool m_ownsData;  // 0x28
};
}

namespace social::cache {
struct Time {  // sizeof 0x8
    Time();
    ~Time();
    void SetToNow();
    long GetSeconds();
    long Diff(const social::cache::Time&);
    long Elapsed();
    bool operator==(const social::cache::Time&);
    bool operator!=(const social::cache::Time&);
    bool operator>(const social::cache::Time&);
    bool operator<(const social::cache::Time&);
private:
    __int64 m_time;  // 0x0
};
}

#endif
