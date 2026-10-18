// libs/gaia/include/utils/threadmanagerrequest.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_THREADMANAGERREQUEST_H
#define LIBS_GAIA_INCLUDE_UTILS_THREADMANAGERREQUEST_H

namespace gaia {
class ThreadManagerRequest {  // sizeof 0xC
public:
    ThreadManagerRequest(gaia::GaiaRequest*);
    ThreadManagerRequest(gaia::AsyncRequestImpl*);
    gaia::AsyncRequestImpl *GetAsyncRequest();
    gaia::GaiaRequest *GetGaiaRequest();
    gaia::ThreadManagerRequest::RequestType GetRequestType();
    int GetPriority();
    gaia::GaiaServices GetGaiaServiceIdentifier(gaia::OpCodes);
    gaia::OpCodes GetOperationCode();
    gaia::OpCodes DetectService();
    void TriggerCallback();
    bool IsCanceled();
private:
    gaia::ThreadManagerRequest::RequestType e_type;  // 0x0
    gaia::AsyncRequestImpl *m_AsyncRequest;  // 0x4
    gaia::GaiaRequest *m_GaiaRequest;  // 0x8
};
}

#endif
