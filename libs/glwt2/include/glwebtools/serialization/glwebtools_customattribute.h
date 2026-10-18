// libs/glwt2/include/glwebtools/serialization/glwebtools_customattribute.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_CUSTOMATTRIBUTE_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_CUSTOMATTRIBUTE_H

namespace glwebtools {
class CustomArgument {  // sizeof 0x24
public:
    CustomArgument(const glwebtools::JSONValue&);
    CustomArgument(const char*);
    CustomArgument(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    CustomArgument(const glwebtools::CustomArgument&);
    CustomArgument();
    glwebtools::CustomArgument &operator=(const glwebtools::CustomArgument&);
    bool operator==(const glwebtools::CustomArgument&);
    int ToString(glwebtools::SecureString&);
    int ToString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &ToString();
    int ToInt(int&);
    int ToInt();
    int ToUInt(unsigned&);
    unsigned ToUInt();
    int ToFloat(float&);
    float ToFloat();
    int ToDouble(double&);
    double ToDouble();
    int ToBool(bool&);
    bool ToBool();
    int operator>>(bool&);
    int operator>>(double&);
    int operator>>(float&);
    int operator>>(unsigned&);
    int operator>>(int&);
    int operator>>(glwebtools::SecureString&);
    int operator>>(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsValid();
    bool IsInt();
    bool IsUInt();
    bool IsFloat();
    bool IsDouble();
    bool IsBool();
    bool IsString();
    bool IsJSON();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_value;  // 0x0
    glwebtools::CustomArgument::Type m_type;  // 0x20
};
}

namespace glwebtools {
class CustomAttribute {  // sizeof 0x44
public:
    CustomAttribute(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const glwebtools::CustomArgument&);
    CustomAttribute(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    CustomAttribute();
    void key(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &key();
    glwebtools::CustomArgument &value();
    const glwebtools::CustomArgument &value();
    bool operator==(const glwebtools::CustomAttribute&);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0x0
    glwebtools::CustomArgument m_value;  // 0x20
};
}

namespace glwebtools {
class CustomAttributeList {  // sizeof 0x1C
public:
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::const_iterator begin();
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::iterator begin();
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::const_iterator end();
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::iterator end();
    bool operator==(const glwebtools::CustomAttributeList&);
    bool empty();
    unsigned size();
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::iterator insert(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const glwebtools::CustomArgument&);
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::iterator insert(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::iterator insert(const glwebtools::CustomAttribute&);
    std::_Tree<std::_Tset_traits<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4>,0> >::const_iterator find(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void erase(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void swap(glwebtools::CustomAttributeList&);
    void clear();
    unsigned count(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const glwebtools::CustomArgument &operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    std::set<glwebtools::CustomAttribute,std::less<glwebtools::CustomAttribute>,glwebtools::SAllocator<glwebtools::CustomAttribute,4> > m_attributes;  // 0x0
    static const glwebtools::CustomArgument InvalidArgument;
};
}

#endif
