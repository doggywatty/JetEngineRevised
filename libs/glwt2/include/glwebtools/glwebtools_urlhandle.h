// libs/glwt2/include/glwebtools/glwebtools_urlhandle.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_GLWEBTOOLS_URLHANDLE_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_GLWEBTOOLS_URLHANDLE_H

namespace glwebtools {
class UrlConnection : public glwebtools::HandleBase<glwebtools::UrlConnectionCore> {  // sizeof 0x8
protected:
    UrlConnection(unsigned);
public:
    UrlConnection(const glwebtools::UrlConnection&);
    UrlConnection();
    virtual ~UrlConnection();
    glwebtools::UrlConnection &operator=(const glwebtools::UrlConnection&);
    int Release();
    glwebtools::UrlConnection::State GetState();
    glwebtools::UrlResponse GetUrlResponse();
    int StartRequest(glwebtools::UrlRequest&);
    int CancelRequest();
    bool IsHandleValid();
    bool IsReadyToRun();
    bool IsRunning();
    bool IsError();
    int GetLastError();
    unsigned GetDownloadedBytes();
    bool SupportServerSideEvent();
    bool HasServerSideEvent();
    int PopServerSideEvent(glwebtools::ServerSideEvent&);
    bool SupportDataPacketQueue();
    bool HasDataPacket();
    int PopDataPacket(glwebtools::MutableData&);
    int GetCreationSettings(glwebtools::UrlConnection::CreationSettings&);
};
}

namespace glwebtools {
class UrlRequest : public glwebtools::HandleBase<glwebtools::UrlRequestCore> {  // sizeof 0x8
    UrlRequest(unsigned);
public:
    UrlRequest(const glwebtools::UrlRequest&);
    UrlRequest();
    virtual ~UrlRequest();
    glwebtools::UrlRequest &operator=(const glwebtools::UrlRequest&);
    int Release();
    glwebtools::UrlRequest::Method GetMethod();
    glwebtools::UrlRequest::State GetState();
    int Reset();
    int AddHeaders(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int AddHeaders(const char*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int AddHeaders(const char*, const char*);
    int SetHeaders(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    int ClearHeaders();
    int SetUrl(glwebtools::UrlRequest::Protocol, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    int SetUrl(glwebtools::UrlRequest::Protocol, const char*, const char*, unsigned);
    int SetUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    int SetUrl(const char*, unsigned);
    int SetHTTPUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    int SetHTTPUrl(const char*, const char*, unsigned);
    int SetHTTPSUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    int SetHTTPSUrl(const char*, const char*, unsigned);
    int SetMethod(glwebtools::UrlRequest::Method);
    int SetData(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    int SetData(const void*, unsigned);
    int SetData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int AddData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int AddData(const char*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int AddData(const char*, const char*);
    int AddData(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    int ClearData();
    bool IsHandleValid();
    bool IsInitialState();
    bool IsReadyToRun();
    bool IsRunning();
    bool IsError();
    int GetCreationSettings(glwebtools::UrlRequest::CreationSettings&);
};
}

namespace glwebtools {
class UrlResponse : public glwebtools::HandleBase<glwebtools::UrlConnectionCore> {  // sizeof 0x8
    UrlResponse(unsigned);
public:
    UrlResponse(const glwebtools::UrlResponse&);
    UrlResponse();
    virtual ~UrlResponse();
    glwebtools::UrlResponse &operator=(const glwebtools::UrlResponse&);
    int Release();
    int GetData(const void*&, unsigned&);
    int GetDataAsString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    long GetResponseCode();
    const char *GetHeaderField(const char*);
    bool IsHandleValid();
    bool IsResponseReady();
    bool IsHTTPError();
};
}

#endif
