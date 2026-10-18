// libs/gaia/include/servicemanagers/iris.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_IRIS_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_IRIS_H

namespace gaia {
class Iris : public gaia::BaseServiceManager {  // sizeof 0x128
public:
    Iris(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Iris();
    int getAsset(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, int, int, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, gaia::GaiaRequest*);
    int GetAssetETag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int UploadAsset(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, bool, gaia::GaiaRequest*);
    int GetAssetMetadata(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, void*&, int*, gaia::GaiaRequest*);
    int GetAssetURL(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int CreateCoupons(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const unsigned, const unsigned, const unsigned, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int RedeemCoupon(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
};
}

#endif
