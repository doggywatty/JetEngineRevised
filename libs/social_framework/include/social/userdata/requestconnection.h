// libs/social_framework/include/social/userdata/requestconnection.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUESTCONNECTION_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUESTCONNECTION_H

namespace social {
class RequestConnection : public social::Request {  // sizeof 0x5C
public:
    RequestConnection(social::RequestBox*, social::Request::ERequestStatus, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::UserOsiris*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::UserOsiris*);
    virtual ~RequestConnection();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetConnectionType();
    social::UserOsiris *GetConnector();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_connectionType;  // 0x38
    social::UserOsiris *m_connector;  // 0x58
};
}

#endif
