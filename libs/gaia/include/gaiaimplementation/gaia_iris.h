// libs/gaia/include/gaiaimplementation/gaia_iris.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_IRIS_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_IRIS_H

namespace gaia {
class Gaia_Iris {  // sizeof 0x8
    int GetAccessToken(gaia::GaiaRequest&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    int GetIrisStatus();
    int StartAndAuthorizeIris(gaia::GaiaRequest&);
    int StartAndAuthorizeIris(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetAsset(gaia::GaiaRequest&);
    int GetAsset(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, int, int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetAssetETag(gaia::GaiaRequest&);
    int GetAssetETag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetAssetCheckEtag(gaia::GaiaRequest&);
    int GetAssetCheckEtag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, int, int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetAssetHash(gaia::GaiaRequest&);
    int GetAssetHash(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetAssetSize(gaia::GaiaRequest&);
    int GetAssetSize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int UploadAsset(gaia::GaiaRequest&);
    int UploadAsset(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*, const unsigned, bool, bool, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetAssetURL(gaia::GaiaRequest&);
    int GetAssetURL(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CreateCoupons(gaia::GaiaRequest&);
    int CreateCoupons(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const unsigned, const unsigned, const unsigned, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RedeemCoupon(gaia::GaiaRequest&);
    int RedeemCoupon(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void**, int*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
private:
    glwebtools::Mutex m_serviceMutex;  // 0x0
};
}

#endif
