// libs/offlineitems/include/offlineitems/object/oi_promotion.h
#ifndef LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_PROMOTION_H
#define LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_PROMOTION_H

namespace oi {
class PromotionOI {  // sizeof 0x54
public:
    virtual int read(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual int read(const glwebtools::JsonReader&);
    void Clear();
    virtual ~PromotionOI();
    const char *GetEndTime();
    const char *GetDescription();
    void Print();
private:
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringValidator,glwebtools::AttributeFormatter> m_endTime;  // 0x4
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringValidator,glwebtools::AttributeFormatter> m_description;  // 0x2C
};
}

#endif
