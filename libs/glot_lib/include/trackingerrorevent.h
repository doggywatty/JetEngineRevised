// libs/glot_lib/include/trackingerrorevent.h
#ifndef LIBS_GLOT_LIB_INCLUDE_TRACKINGERROREVENT_H
#define LIBS_GLOT_LIB_INCLUDE_TRACKINGERROREVENT_H

namespace glot {
class TrackingErrorEvent : public glot::TrackingEventBase {  // sizeof 0x30
public:
    TrackingErrorEvent(glot::GlotErrorEventIdCodes, glot::GlotErrorLogLevel, unsigned long, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    TrackingErrorEvent();
    ~TrackingErrorEvent();
    virtual Json::Value &GetData();
    bool Serialize(_iobuf*);
    bool Serialize(const char*);
    bool Deserialize(_iobuf*);
    bool Deserialize(const char*);
    virtual void SetStartOffset(int);
    virtual void SetEndOffset(int);
    virtual int GetEndOffset();
    virtual int GetStartOffSet();
    unsigned long GetGeneratedTime();
    glot::GlotErrorEventIdCodes GetErrorType();
private:
    glot::GlotErrorEventIdCodes m_tracker_id;  // 0x4
    glot::GlotErrorLogLevel m_level;  // 0x8
    unsigned long m_timestamp;  // 0xC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_error;  // 0x10
};
}

#endif
