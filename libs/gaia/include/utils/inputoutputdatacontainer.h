// libs/gaia/include/utils/inputoutputdatacontainer.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_INPUTOUTPUTDATACONTAINER_H
#define LIBS_GAIA_INCLUDE_UTILS_INPUTOUTPUTDATACONTAINER_H

namespace gaia {
class InputOutputDataContainer {  // sizeof 0xC0
public:
    InputOutputDataContainer();
    ~InputOutputDataContainer();
    void SetResponse(const std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    void SetResponse(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetResponse(void*, int&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > Serialize();
    Json::Value m_params;  // 0x0
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_CustomParams;  // 0x10
    int m_statusCode;  // 0x2C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_AccessToken;  // 0x30
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_ServiceScope;  // 0x50
    gaia::HermesBaseMessage *m_BaseMessage;  // 0x70
    gaia::OpCodes m_eOperationCode;  // 0x74
    bool m_isAsync;  // 0x78
    void* caller;  // 0x7C
    void* m_pUserDataPtr;  // 0x80
    void* m_pData;  // 0x84
    int m_pDataSize;  // 0x88
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_DataString;  // 0x8C
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_jsonArray;  // 0xAC
};
}

#endif
