// libs/gaia/include/utils/gaiarequest.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_GAIAREQUEST_H
#define LIBS_GAIA_INCLUDE_UTILS_GAIAREQUEST_H

namespace gaia {
class GaiaRequest {  // sizeof 0x3C
public:
    GaiaRequest(const gaia::GaiaRequest&);
    GaiaRequest();
    ~GaiaRequest();
    void DeleteReferences();
    gaia::GaiaRequest &operator=(const gaia::GaiaRequest&);
    void Grab();
    void Drop();
private:
    void SetRunAsynchronous(void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
public:
    void SetRunAsynchronous(void (gaia::GaiaRequest)*, void*);
    int TriggerCallback();
    void SetCustomParams(const std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    void SetUserData(void*);
    void GetUserData(void*&);
    void* GetCaller();
    void SetAccessToken(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetHermesBaseMessage(const gaia::HermesBaseMessage*);
    Json::Value &operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetKairosCallback(void*);
    void SetPriority(gaia::GaiaRequest::ePriority);
    int GetPriority();
    gaia::GaiaRequest::eResponseType GetResponseType();
    int GetResponse(std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    int GetResponse(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetResponse(void*&, int&);
    int GetResponseCode();
    gaia::OpCodes GetOperationCode();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > Serialize();
    __int64 GetID();
    void Cancel();
    bool IsCanceled();
private:
    int ValidateMandatoryParam(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::ValueType);
    int ValidateOptionalParam(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::ValueType);
    bool isValid();
    void SetAsyncOperation(bool);
    void GetInputValue(std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    Json::Value GetInputValue(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    Json::Value GetInputValue(const char*);
    void SetResponse(const std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    void SetResponse(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetResponse(void*, int&);
    void SetResponseCode(int);
    void SetOperationCode(gaia::OpCodes);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    gaia::HermesBaseMessage *GetHermesBaseMessage();
    void SetParamsForOutput(std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    void SetParamsForOutput(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetParamsForOutput(void*&, int&);
    bool isAsyncOperation();
    static __int64 MakeID();
protected:
    gaia::InputOutputDataContainer *m_dataPtr;  // 0x0
    glwebtools::Mutex *m_mutex;  // 0x4
    int* m_refCount;  // 0x8
    void (*m_callbackFct)(gaia::GaiaRequest);  // 0xC
    void* m_kairosCallback;  // 0x10
    bool* m_isCanceled;  // 0x14
    __int64* m_ID;  // 0x18
    int* m_priority;  // 0x1C
    static glwebtools::Mutex s_IDMutex;
    static __int64 s_IDGenerator;
    void (*m_old_callbackFct)(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);  // 0x20
    void* *m_dataPtrOutput;  // 0x24
    int* m_dataSizeOutput;  // 0x28
    bool* m_isValid;  // 0x2C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > *m_stringOutput;  // 0x30
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > *m_jsonArrayOutput;  // 0x34
    gaia::GaiaRequest::eResponseType *m_eResponseType;  // 0x38
};
}

#endif
