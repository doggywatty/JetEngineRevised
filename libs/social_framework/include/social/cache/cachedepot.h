// libs/social_framework/include/social/cache/cachedepot.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEDEPOT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_CACHE_CACHEDEPOT_H

namespace social::cache {
class CacheDepot {  // sizeof 0xA0
public:
    CacheDepot(social::cache::CacheRequestManager&);
    CacheDepot();
    ~CacheDepot();
    social::cache::Error Initialize(const social::cache::CacheDepot::CreationSettings&);
    social::cache::CacheDepotStatus GetStatus();
private:
    social::cache::Error Update(social::cache::CacheObjectHandle&, const social::cache::CacheObjectData&);
    social::cache::Error Update(social::cache::CacheObjectHandle&, const char*, unsigned);
public:
    void Update();
    bool Has(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::CacheObjectHandle Get(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::vector<social::cache::CacheObjectHandle,std::allocator<social::cache::CacheObjectHandle> > GetObjects();
    social::cache::Error Store(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const social::cache::CacheObjectData&);
    social::cache::Error Store(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, unsigned);
    social::cache::Error EraseAll();
    social::cache::Error UnloadAll();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetKeyPath(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetPath();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetName();
    unsigned GetObjectCount();
    const social::cache::CacheInfo &GetInfo();
    social::cache::CacheDiscardAlgorithm GetDiscardAlgorithm();
private:
    social::cache::Error Load(social::cache::CacheObjectHandle&);
    social::cache::Error Unload(const social::cache::CacheObjectHandle&);
    social::cache::Error Erase(const social::cache::CacheObjectHandle&);
    social::cache::Error Touch(social::cache::CacheObjectHandle&);
    social::cache::CacheRequestHandle FindRequestForObjectKey(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::CacheObject *FindCachedObject(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::Error AddNewCachedObject(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const social::cache::CacheObjectData&);
    void AddObject(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::cache::CacheObject&);
    void UnloadObject(social::cache::CacheObject&);
    void ReleaseObject(social::cache::CacheObject*);
    social::cache::Error EraseObject(social::cache::CacheObject&);
    social::cache::Error InitializeDepotFromDisk(const social::cache::CacheDepot::CreationSettings&);
    void UpdateObject(social::cache::CacheObject&, const social::cache::CacheObjectData&);
    void PushRequestForObject(social::cache::CacheObject&, const social::cache::CacheRequest::CreationSettings&);
    void InitDiscardAlgorithm();
    void OnObjectLoaded(const social::cache::CacheObject&);
    void OnObjectUnloaded(const social::cache::CacheObject&);
    void OnObjectErase(const social::cache::CacheObject&);
    void OnObjectErased(unsigned);
    void OnObjectAdded(const social::cache::CacheObject&);
    void OnObjectUpdate(const social::cache::CacheObject&, const social::cache::CacheObjectData&);
    void OnRequestStarted(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const social::cache::CacheRequest&);
    void OnRequestCompleted(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const social::cache::CacheRequest&);
    social::cache::Error ValidateSettings(const social::cache::CacheDepot::CreationSettings&);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::cache::CacheObject *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::cache::CacheObject *> > > m_objects;  // 0x0
    social::cache::CacheRequestManager *m_requestMgr;  // 0x1C
    social::cache::CacheDepotStatus m_status;  // 0x20
    social::cache::CacheDepot::CreationSettings m_settings;  // 0x24
    bool m_ownsRequestMgr;  // 0x6C
    social::cache::CacheInfo m_info;  // 0x70
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::list<social::cache::CacheRequestHandle,std::allocator<social::cache::CacheRequestHandle> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::list<social::cache::CacheRequestHandle,std::allocator<social::cache::CacheRequestHandle> > > > > m_objectRequests;  // 0x80
    social::cache::ICacheDiscardAlgorithm *m_discardAlgorithm;  // 0x9C
};
}

namespace social::cache {
class CacheDepotHandle {  // sizeof 0x24
protected:
    CacheDepotHandle(social::cache::CacheManager&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    CacheDepotHandle();
    ~CacheDepotHandle();
    bool IsValid();
    bool IsInitialized();
    bool Has(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::cache::CacheObjectHandle Get(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::vector<social::cache::CacheObjectHandle,std::allocator<social::cache::CacheObjectHandle> > GetObjects();
    social::cache::Error Load(social::cache::CacheObjectHandle&);
    social::cache::Error Unload(const social::cache::CacheObjectHandle&);
    social::cache::Error Store(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const social::cache::CacheObjectData&);
    social::cache::Error Store(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, unsigned);
    social::cache::Error Update(social::cache::CacheObjectHandle&, const social::cache::CacheObjectData&);
    social::cache::Error Update(social::cache::CacheObjectHandle&, const char*, unsigned);
    social::cache::Error Erase(const social::cache::CacheObjectHandle&);
    social::cache::Error Touch(social::cache::CacheObjectHandle&);
    social::cache::Error EraseAll();
    social::cache::Error UnloadAll();
    unsigned GetObjectCount();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetPath();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetName();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetKeyPath(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const social::cache::CacheInfo &GetInfo();
protected:
    social::cache::CacheDepot *GetDepot();
private:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_empty_string;
    static const social::cache::CacheInfo k_invalid_info;
    social::cache::CacheManager *m_manager;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0x4
};
}

namespace social::cache {
class FIFODiscardAlgorithm : public social::cache::ICacheDiscardAlgorithm {  // sizeof 0x1C
public:
    virtual void RegisterObject(social::cache::CacheObject*);
    virtual void UnregisterObject(const social::cache::CacheObject*);
    virtual unsigned GetRegisteredObjectCount();
    virtual unsigned GetCandidatesToReleaseForSize(unsigned, std::vector<social::cache::CacheObject *,std::allocator<social::cache::CacheObject *> >&);
private:
    std::list<social::cache::CacheObject *,std::allocator<social::cache::CacheObject *> > m_objects;  // 0x4
};
}

namespace social::cache {
class RLUDiscardAlgorithm : public social::cache::ICacheDiscardAlgorithm {  // sizeof 0x20
public:
    RLUDiscardAlgorithm();
    virtual ~RLUDiscardAlgorithm();
    virtual void RegisterObject(social::cache::CacheObject*);
    virtual void UnregisterObject(const social::cache::CacheObject*);
    virtual unsigned GetRegisteredObjectCount();
    virtual unsigned GetCandidatesToReleaseForSize(unsigned, std::vector<social::cache::CacheObject *,std::allocator<social::cache::CacheObject *> >&);
private:
    void SortIfNeeded();
    std::list<social::cache::CacheObject *,std::allocator<social::cache::CacheObject *> > m_objects;  // 0x4
    bool m_isSorted;  // 0x1C
};
}

#endif
