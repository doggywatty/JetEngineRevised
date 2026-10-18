// libs/gaia/include/servicemanagers/baseservicemanager.h
#ifndef LIBS_GAIA_INCLUDE_SERVICEMANAGERS_BASESERVICEMANAGER_H
#define LIBS_GAIA_INCLUDE_SERVICEMANAGERS_BASESERVICEMANAGER_H

namespace gaia {
class BaseServiceManager {  // sizeof 0x128
public:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetCredentialString(const gaia::BaseServiceManager::Credentials&);
    static gaia::BaseServiceManager::Credentials GetCredentialFromString(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual void CompleteRequest(const glwebtools::UrlConnection&, gaia::ServiceRequest*);
    virtual bool StartRequest(glwebtools::UrlConnection&, gaia::ServiceRequest*);
    virtual gaia::State GetState(glwebtools::UrlConnection&);
    BaseServiceManager(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    virtual ~BaseServiceManager();
    virtual void Update();
    void appendEncodedParams(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const unsigned&, bool);
    void appendEncodedParams(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const int&, bool);
    void appendEncodedParams(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual bool NeedsLinkRefreshing();
    void UpdateServiceUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    unsigned long GetTimeStamp();
    void ForceRefreshLink();
    void ResetUsageCount();
    int SendCompleteRequest(gaia::ServiceRequest*);
    int SendCompleteRequest(gaia::ServiceRequest*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SendCompleteRequest(gaia::ServiceRequest*, void*&, int*);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
    int CancelRequest(gaia::OpCodes);
    static int ParseMessages(void*, int, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, gaia::BaseJSONServiceResponse::JSONServiceResponseType);
    void UpdateRunningRequests();
    void StartNewRequests();
    void DeleteUnusedConnections();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetServiceUrl();
protected:
    virtual void setServerDate(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > getServerDate();
    virtual void setFinalServicesUrls(char*, gaia::ServiceRequest*);
    virtual void ProcessServerSideEvent(glwebtools::UrlConnection&);
    gaia::ServiceRequest *GetNextRequest();
    std::queue<gaia::ServiceRequest *,std::deque<gaia::ServiceRequest *,std::allocator<gaia::ServiceRequest *> > > m_requests;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_serviceUrl;  // 0x20
    unsigned m_serviceUsageCount;  // 0x40
    long m_serviceUrlTimestamp;  // 0x44
    bool m_needsLinkRefreshing;  // 0x48
    glwebtools::UrlConnection::CreationSettings m_cs;  // 0x4C
    glwebtools::GlWebTools *m_glwtInstance;  // 0xEC
    glwebtools::Mutex m_mutex;  // 0xF0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_clientID;  // 0xF8
    int m_NrActiveConnections;  // 0x118
    int m_NrMaxConnections;  // 0x11C
    glwebtools::UrlConnection *m_connections;  // 0x120
    gaia::ServiceRequest **m_runningRequests;  // 0x124
};
}

#endif
