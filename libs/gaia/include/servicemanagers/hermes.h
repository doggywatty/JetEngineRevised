// libs/gaia/include/servicemanagers/hermes.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_HERMES_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_HERMES_H

namespace gaia {
class Hermes : public gaia::BaseServiceManager {  // sizeof 0x128
public:
    Hermes(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~Hermes();
    int RegisterEndpoint(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int SendMessageToUser(gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::HermesBaseMessage*, void*, const int, int, bool, gaia::GaiaRequest*);
    int SendMessageToMultipleUsers(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::HermesBaseMessage*, void*, const int, gaia::GaiaRequest*);
    int RetrieveMessages(gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, bool, gaia::GaiaRequest*);
    int RetrieveMessage(gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int DeleteMessage(gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
    int UpdateListSubscription(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, gaia::GaiaRequest*);
    int ShowSubscriptions(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int ListRegisteredDevices(gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, int*, gaia::GaiaRequest*);
    int DeleteAllMessages(gaia::Hermes::TransportTypes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, gaia::GaiaRequest*);
private:
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > CreateTransportsVector();
    static const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > s_transportsVector;
};
}

#endif
