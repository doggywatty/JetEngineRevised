// libs/glot_lib/include/trackingmessage.h
#ifndef LIBS_GLOT_LIB_INCLUDE_TRACKINGMESSAGE_H
#define LIBS_GLOT_LIB_INCLUDE_TRACKINGMESSAGE_H

namespace glot {
class TrackingMessage {  // sizeof 0x18
public:
    TrackingMessage(unsigned, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool AddEventToMessage(glot::TrackingEventBase*);
    int GetEventsCount();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetDataString();
    Json::Value GetData();
    int GetMessageID();
private:
    Json::Value m_data;  // 0x0
    static Json::FastWriter m_jWriter;
    int m_evCout;  // 0x10
};
}

#endif
