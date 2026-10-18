// libs/social_framework/include/social/userdata/binarydataseshat.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_BINARYDATASESHAT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_BINARYDATASESHAT_H

namespace social {
class BinaryDataSeshat : public social::BinaryData {  // sizeof 0x108
    BinaryDataSeshat(const social::BinaryDataSeshat&);
public:
    BinaryDataSeshat(const social::User*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~BinaryDataSeshat();
    virtual void SetData(const char*, int, social::BinaryData::Visibility);
    virtual void Load(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
protected:
    virtual void SaveImpl();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void sOnDataLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnDataSaved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_loadEtag;  // 0xE8
};
}

#endif
