// libs/gaia/include/serviceresponseclasses/basejsonserviceresponse.h
#ifndef LIBS_GAIA_INCLUDE_SERVICERESPONSECLASSES_BASEJSONSERVICERESPONSE_H
#define LIBS_GAIA_INCLUDE_SERVICERESPONSECLASSES_BASEJSONSERVICERESPONSE_H

namespace gaia {
class BaseJSONServiceResponse {  // sizeof 0x18
public:
    BaseJSONServiceResponse(Json::Value, gaia::BaseJSONServiceResponse::JSONServiceResponseType);
    BaseJSONServiceResponse(Json::Value);
    BaseJSONServiceResponse();
    int GetType();
    void SetType(gaia::BaseJSONServiceResponse::JSONServiceResponseType);
    ~BaseJSONServiceResponse();
    const Json::Value &GetJSONMessage();
private:
    Json::Value m_message;  // 0x0
    gaia::BaseJSONServiceResponse::JSONServiceResponseType m_type;  // 0x10
};
}

#endif
