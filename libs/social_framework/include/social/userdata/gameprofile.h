// libs/social_framework/include/social/userdata/gameprofile.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_GAMEPROFILE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_GAMEPROFILE_H

namespace social {
class GameProfile : public social::MultiStorable {  // sizeof 0x274
public:
    GameProfile(social::UserOsiris*);
    virtual ~GameProfile();
    static void RegisterKey(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const social::GameProfileEntry &Get(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, float, social::GameProfileEntry::EVisibility);
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, social::GameProfileEntry::EVisibility);
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::GameProfileEntry::EVisibility);
    void Append(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::GameProfileEntry::EVisibility);
private:
    static void sOnProfileLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
protected:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    static std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > s_gameProfileKeys;
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::GameProfileEntry *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::GameProfileEntry *> > > m_profileEntries;  // 0x174
    social::GameProfileEntry m_emptyEntry;  // 0x190
};
}

namespace social {
class GameProfileEntry : public social::Storable {  // sizeof 0xE4
public:
    GameProfileEntry(social::UserOsiris*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~GameProfileEntry();
    virtual social::Batcher *GetBatch();
    void OnValueLoaded(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    void SetValue(float, social::GameProfileEntry::EVisibility);
    void SetValue(int, social::GameProfileEntry::EVisibility);
    void SetValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::GameProfileEntry::EVisibility);
    void AppendValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::GameProfileEntry::EVisibility);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &AsString();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &operator const class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > &();
    int AsInt();
    int operator int();
    float AsFloat();
    float operator float();
    void SetSaved(bool);
    social::GameProfileEntry::EVisibility GetVisibility();
    social::GameProfileEntry::EOperation GetOperation();
protected:
    virtual void SaveImpl();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void sOnProfileSaved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnVisibilitySaved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    social::GameProfileEntry::EVisibility m_visibility;  // 0xBC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_value;  // 0xC0
    social::GameProfileEntry::EOperation m_operation;  // 0xE0
};
}

#endif
