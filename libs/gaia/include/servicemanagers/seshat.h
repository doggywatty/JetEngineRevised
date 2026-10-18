// libs/gaia/include/servicemanagers/seshat.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_SESHAT_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_SESHAT_H

namespace gaia {
class Seshat : public gaia::BaseServiceManager {  // sizeof 0x144
public:
    Seshat(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Seshat();
    int PutDataCheckEtag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Seshat::dataVisibility, gaia::GaiaRequest*);
    int GetDataCheckEtag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int GetEtagForKey(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int PutData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Seshat::dataVisibility, gaia::GaiaRequest*);
    int GetData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int DeleteData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int SetProfile(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int GetProfile(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int DeleteProfile(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int SetProfileVisibility(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int GetProfileVisibility(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int GetBatchProfiles(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int CreateMatcher(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int GetMatches(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, unsigned, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >*, gaia::GaiaRequest*);
    int DeleteMatcher(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
private:
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > CreateVisibilityVector();
public:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_visibility_private;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_visibility_public_read;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_visibility_public_write;
private:
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > etagMap;  // 0x128
    static const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > s_visibilityVector;
};
}

#endif
