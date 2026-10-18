// libs/glot_lib/include/trackingevent.h
#ifndef LIBS_GLOT_LIB_INCLUDE_TRACKINGEVENT_H
#define LIBS_GLOT_LIB_INCLUDE_TRACKINGEVENT_H

namespace glot {
struct EventValue {  // sizeof 0x10
    int operator int();
    unsigned operator unsigned int();
    const char *operator const char *();
    bool operator bool();
    double operator double();
    bool operator==(const glot::EventValue&);
    Json::Value operator class Json::Value();
    EventValue(const glot::EventValue&);
    EventValue(const double);
    EventValue(const bool);
    EventValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    EventValue(const char*);
    EventValue(const unsigned);
    EventValue(const int);
    glot::EventValue::ValueType GetType();
    ~EventValue();
    glot::EventValue::EventData data;  // 0x0
private:
    glot::EventValue::ValueType type;  // 0x8
};
}

namespace glot {
class TrackingEvent : public glot::TrackingEventBase {  // sizeof 0x28
public:
    TrackingEvent(int, glot::TrackingEvent::eventType);
    TrackingEvent(int);
    TrackingEvent();
    ~TrackingEvent();
    int SetData(const Json::Value&);
    virtual Json::Value &GetData();
    int GetEventId();
    int SetType(glot::TrackingEvent::eventType);
    glot::TrackingEvent::eventType GetType();
    bool Deserialize(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_fstream<char,std::char_traits<char> >&);
    virtual void SetStartOffset(int);
    virtual void SetEndOffset(int);
    virtual int GetEndOffset();
    virtual int GetStartOffSet();
private:
    int m_StartOffset;  // 0x4
    int m_EndOffset;  // 0x8
    int m_id;  // 0xC
    glot::TrackingEvent::eventType m_type;  // 0x10
    Json::Value m_data;  // 0x18
};
}

#endif
