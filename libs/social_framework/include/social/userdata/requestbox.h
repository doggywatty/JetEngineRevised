// libs/social_framework/include/social/userdata/requestbox.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUESTBOX_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_REQUESTBOX_H

namespace social {
class RequestBox : public social::Storable {  // sizeof 0x114
public:
    RequestBox(social::UserOsiris*);
    virtual ~RequestBox();
    void CancelRequest(social::Request*);
    void AcceptRequest(social::Request*);
    void IgnoreRequest(social::Request*);
    void RejectRequest(social::Request*);
    void SendRequest(social::UserOsiris*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SendFriendRequest(social::UserOsiris*);
    std::_Vector_const_iterator<social::Request *,std::allocator<social::Request *> > begin();
    std::_Vector_iterator<social::Request *,std::allocator<social::Request *> > begin();
    std::_Vector_const_iterator<social::Request *,std::allocator<social::Request *> > end();
    unsigned size();
    std::_Vector_const_iterator<social::Request *,std::allocator<social::Request *> > sent_begin();
    std::_Vector_iterator<social::Request *,std::allocator<social::Request *> > sent_begin();
    std::_Vector_const_iterator<social::Request *,std::allocator<social::Request *> > sent_end();
    unsigned sent_size();
protected:
    virtual void SaveImpl();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void sOnRequests(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnSentRequests(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnRequestSaved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
    std::vector<social::Request *,std::allocator<social::Request *> > m_receivedRequests;  // 0xBC
    std::vector<social::Request *,std::allocator<social::Request *> > m_sentRequests;  // 0xD0
    std::vector<social::Request *,std::allocator<social::Request *> > m_outputRequests;  // 0xE4
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_requestResponses;  // 0xF8
    unsigned m_savingRequests;  // 0x10C
    bool m_errorDuringSave;  // 0x110
};
}

#endif
