// libs/social_framework/include/social/userdata/request.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUEST_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUEST_H

namespace social {
class Request {  // sizeof 0x38
public:
    virtual ~Request();
    void Accept();
    void Reject();
    void Ignore();
    void Cancel();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetId();
    social::UserOsiris *GetRequester();
    social::Request::ERequestType GetRequestType();
    social::Request::ERequestStatus GetRequestStatus();
    social::Request::EResponse GetResponse();
protected:
    Request(social::RequestBox*, social::Request::ERequestType, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::UserOsiris*, social::Request::ERequestStatus);
    void SetResponse(social::Request::EResponse);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_id;  // 0x4
    social::UserOsiris *m_requester;  // 0x24
    social::Request::ERequestType m_requestType;  // 0x28
    social::Request::ERequestStatus m_status;  // 0x2C
    social::RequestBox *m_box;  // 0x30
    social::Request::EResponse m_response;  // 0x34
};
}

#endif
