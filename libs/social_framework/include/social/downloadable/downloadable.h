// libs/social_framework/include/social/downloadable/downloadable.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_DOWNLOADABLE_DOWNLOADABLE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_DOWNLOADABLE_DOWNLOADABLE_H

namespace social::downloadable {
class Downloadable : public social::RefCounted, public social::SimpleEventDispatcher<void *> {  // sizeof 0xA8
public:
    virtual ~Downloadable();
    social::downloadable::Error Download();
    void Update();
    social::downloadable::Downloadable::Type GetType();
    social::downloadable::DownloadableStatus GetStatus();
    social::downloadable::Error GetData(const void*&, unsigned&);
    social::downloadable::Error GetEtag(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    Downloadable(social::downloadable::Downloadable::Type, const social::downloadable::ResourceDescriptor&, const social::cache::CacheObjectHandle&);
    social::downloadable::Downloadable::IDownloadOperation *CreateDownloadOperation();
    social::downloadable::Downloadable::Type m_type;  // 0x28
    social::downloadable::ResourceDescriptor m_rd;  // 0x2C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_cacheKey;  // 0x50
    social::cache::CacheObjectHandle m_cacheObjectHandle;  // 0x70
    social::downloadable::Downloadable::IDownloadOperation *m_downloadOperation;  // 0xA0
    social::downloadable::DownloadableStatus m_status;  // 0xA4
};
}

#endif
