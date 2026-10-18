// libs/social_framework/include/social/cache/cacheobject.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEOBJECT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEOBJECT_H

namespace social::cache {
class CacheObject : public social::SimpleEventDispatcher<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const &> {  // sizeof 0x78
public:
    CacheObject(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const social::cache::CacheObjectData&);
    CacheObject(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    ~CacheObject();
    void Touch();
    void ReleaseData();
    unsigned GetSize();
    social::cache::Time GetLastAccessTime();
private:
    social::cache::CacheObjectData m_data;  // 0x20
    social::cache::CacheObjectStatus m_status;  // 0x4C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0x50
    social::cache::Time m_lastAccess;  // 0x70
};
}

namespace social::cache {
class CacheObjectHandle {  // sizeof 0x30
    CacheObjectHandle(social::cache::CacheDepot&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    CacheObjectHandle(const social::cache::CacheObjectHandle&);
    CacheObjectHandle();
    ~CacheObjectHandle();
    bool IsValid();
    bool HasRequest();
    social::cache::CacheRequestHandle GetRequest();
    social::cache::CacheObjectStatus GetStatus();
    social::cache::Time GetLastAccessTime();
    const char *GetData();
    unsigned GetSize();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetEtag();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetPath();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetKey();
    bool RegisterEventListener(int, bool (void*, bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&)*, void*);
    bool UnregisterListener(void*);
    bool operator==(const social::cache::CacheObjectHandle&);
    social::cache::Error Load();
    social::cache::Error Unload();
    social::cache::Error Update(const social::cache::CacheObjectData&);
    social::cache::Error Update(const char*, unsigned);
    social::cache::Error Store(const social::cache::CacheObjectData&);
    social::cache::Error Erase();
    social::cache::Error Touch();
private:
    void SetRequest(const social::cache::CacheRequestHandle&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_objectKey;  // 0x0
    social::cache::CacheDepot *m_depot;  // 0x20
    social::cache::CacheRequestHandle m_requestHandle;  // 0x24
};
}

#endif
