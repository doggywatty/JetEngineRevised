// libs/social_framework/include/social/userdata/requestgroup.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUESTGROUP_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUESTGROUP_H

namespace social {
class RequestGroup : public social::Request {  // sizeof 0x3C
public:
    RequestGroup(social::RequestBox*, social::Request::ERequestStatus, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::UserOsiris*, social::Group*, bool);
    virtual ~RequestGroup();
    social::Group *GetGroup();
private:
    social::Group *m_group;  // 0x38
};
}

#endif
