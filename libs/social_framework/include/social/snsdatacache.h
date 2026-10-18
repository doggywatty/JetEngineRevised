// libs/social_framework/include/social/snsdatacache.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_SNSDATACACHE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_SNSDATACACHE_H

namespace social {
class SNSDataCache {  // sizeof 0x74
public:
    SNSDataCache();
    virtual ~SNSDataCache();
    void Reset();
    void SetData(social::TSNSData, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    void SetData(social::TSNSData, std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    void SetData(social::TSNSData, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void AddData(std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > >);
    void AddData(social::TSNSData, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    bool GetData(std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > >&);
    bool GetData(social::TSNSData, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    bool GetData(social::TSNSData, std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool GetData(social::TSNSData, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > > &GetFriendData();
    bool IsDataCachedSimple(social::TSNSData);
    bool IsDataCachedVector(social::TSNSData);
    bool IsDataCachedMap(social::TSNSData);
    bool IsDataCached(social::TSNSData);
    void ClearData(social::TSNSData);
    void ClearAllData();
private:
    std::map<enum social::TSNSData,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<enum social::TSNSData>,std::allocator<std::pair<enum social::TSNSData const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_cache;  // 0x4
    std::map<enum social::TSNSData,std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,std::less<enum social::TSNSData>,std::allocator<std::pair<enum social::TSNSData const ,std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > m_vectorCache;  // 0x20
    std::map<enum social::TSNSData,std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >,std::less<enum social::TSNSData>,std::allocator<std::pair<enum social::TSNSData const ,std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > > m_mapCache;  // 0x3C
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > > m_friendData;  // 0x58
};
}

#endif
