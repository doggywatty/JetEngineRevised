// libs/gaia/include/gaiaimplementation/gaia_hermes.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_HERMES_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_HERMES_H

namespace gaia {
class Gaia_Hermes {  // sizeof 0x8
    int GetAccessToken(gaia::GaiaRequest&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    int GetHermesStatus();
    int StartAndAuthorizeHermes(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int StartAndAuthorizeHermes(gaia::GaiaRequest&);
    int RegisterEndpoint(gaia::GaiaRequest&);
    int RegisterEndpoint(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Hermes::TransportTypes, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SendMessageToUser(gaia::GaiaRequest&);
    int SendMessageToUser(gaia::BaseServiceManager::Credentials, gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, gaia::HermesBaseMessage*, void*, const int, bool, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SendMessageToMultipleUsers(gaia::GaiaRequest&);
    int SendMessageToMultipleUsers(gaia::BaseServiceManager::Credentials, const Json::Value&, gaia::HermesBaseMessage*, void*, const int, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrieveMessages(gaia::GaiaRequest&);
    int RetrieveMessages(gaia::BaseServiceManager::Credentials, gaia::Hermes::TransportTypes, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RetrieveMessage(gaia::GaiaRequest&);
    int RetrieveMessage(gaia::BaseServiceManager::Credentials, gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DeleteMessage(gaia::GaiaRequest&);
    int DeleteMessage(gaia::BaseServiceManager::Credentials, gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DeleteAllMessages(gaia::GaiaRequest&);
    int DeleteAllMessages(gaia::BaseServiceManager::Credentials, gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SubscribeToList(gaia::GaiaRequest&);
    int SubscribeToList(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int UnSubscribeFromList(gaia::GaiaRequest&);
    int UnSubscribeFromList(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ShowSubscriptions(gaia::GaiaRequest&);
    int ShowSubscriptions(gaia::BaseServiceManager::Credentials, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int ListRegisteredDevices(gaia::GaiaRequest&);
    int ListRegisteredDevices(gaia::BaseServiceManager::Credentials, gaia::Hermes::TransportTypes, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
    int CheckActiveNotification();
private:
    int UpdateListSubscription(gaia::GaiaRequest&);
    int UpdateListSubscription(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    glwebtools::Mutex m_serviceMutex;  // 0x0
};
}

#endif
