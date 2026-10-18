// libs/social_framework/include/social/userdata/scoresns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_SCORESNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_SCORESNS_H

namespace social {
class ScoreSNS : public social::Storable {  // sizeof 0xE4
public:
    ScoreSNS(social::UserSNS*);
    virtual ~ScoreSNS();
    void SetCategory(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetValue(int);
    int GetValue();
protected:
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
    virtual void SaveImpl();
private:
    static bool sOnScoreSaved(void*, bool, sociallib::ClientSNSEnum);
    sociallib::ClientSNSEnum m_sns;  // 0xBC
    int m_value;  // 0xC0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_category;  // 0xC4
};
}

#endif
