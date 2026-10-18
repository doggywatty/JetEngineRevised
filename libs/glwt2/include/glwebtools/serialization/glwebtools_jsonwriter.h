// libs/glwt2/include/glwebtools/serialization/glwebtools_jsonwriter.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_JSONWRITER_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_JSONWRITER_H

namespace glwebtools {
class JsonWriter {  // sizeof 0x18
    JsonWriter(glwebtools::Json::Value&);
public:
    JsonWriter();
    int write(const glwebtools::JSONArray&);
    int write(const glwebtools::JSONObject&);
    int write(glwebtools::IndexedValue<glwebtools::JSONArray const >);
    int write(glwebtools::NameValuePair<glwebtools::JSONObject const >);
    int write(const glwebtools::JSONValue&);
    int write(const glwebtools::CustomAttributeList&);
    int write(const glwebtools::CustomArgument&);
    int write(const char*);
    int write(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int write(bool);
    int write(float);
    int write(double);
    int write(unsigned);
    int write(int);
    int include(unsigned*, unsigned*, const glwebtools::JSONArray&);
    int include(const char**, const char**, const glwebtools::JSONObject&);
    int include(const char**, const char**, const glwebtools::CustomAttributeList&);
    int exclude(unsigned*, unsigned*, const glwebtools::JSONArray&);
    int exclude(const char**, const char**, const glwebtools::JSONObject&);
    int exclude(const char**, const char**, const glwebtools::CustomAttributeList&);
    glwebtools::JsonWriter operator[](const char*);
    glwebtools::JsonWriter operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    glwebtools::JsonWriter operator[](unsigned);
    bool IsValid();
    bool isInt();
    bool isUInt();
    bool isDouble();
    bool isBool();
    bool isString();
    bool isObject();
    bool isArray();
    void ToString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString();
private:
    const glwebtools::Json::Value &GetRoot();
    glwebtools::Json::Value &GetRoot();
    glwebtools::Json::Value m_root;  // 0x0
    glwebtools::Json::Value *m_currentRoot;  // 0x10
};
}

#endif
