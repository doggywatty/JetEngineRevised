// libs/social_framework/include/social/utils.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_UTILS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_UTILS_H

namespace social {
struct CustomAttributes : public social::RefCounted {  // sizeof 0x2C
    CustomAttributes(const social::CustomAttributes&);
    CustomAttributes();
    virtual ~CustomAttributes();
    social::CustomAttributes::CustomAttribute &operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool Has(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetStringValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    unsigned GetSize();
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > *GenerateStringMap();
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > *GenerateEncodedStringMap();
    static void MergeReplace(social::CustomAttributes&, const social::CustomAttributes&);
    static void MergeAdd(social::CustomAttributes&, const social::CustomAttributes&);
private:
    const social::CustomAttributes::CustomAttribute &Get(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::CustomAttributes::CustomAttribute,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::CustomAttributes::CustomAttribute> > > m_attributeMap;  // 0x8
    social::IntrusivePointer<social::CustomAttributes::StringMap,social::IntrusivePointerNoLock> m_stringMap;  // 0x24
};
}

namespace social {
class RefCounted {  // sizeof 0x8
public:
    RefCounted();
    virtual ~RefCounted();
    virtual unsigned Grab();
    virtual unsigned Drop();
    int GetRefCount();
private:
    int m_refCount;  // 0x4
};
}

namespace social {
class Utils {  // sizeof 0x1
public:
    static void Base64Encode(const char*, int, char*&, int&);
    static void Base64Decode(const char*, int, char*&, int&);
    static gaia::BaseServiceManager::Credentials ToGaia(sociallib::ClientSNSEnum);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > SnsToString(sociallib::ClientSNSEnum);
    static sociallib::ClientSNSEnum StringtoSns(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetCredentialStr(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, sociallib::ClientSNSEnum);
    static unsigned long ConvertTimeStringToSec(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
};
}

#endif
