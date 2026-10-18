// libs/offlineitems/include/offlineitems/object/oi_itemprices.h
#ifndef LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_ITEMPRICES_H
#define LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_ITEMPRICES_H

namespace oi {
class ItemPrice {  // sizeof 0x48
public:
    virtual int read(const glwebtools::JsonReader&);
    virtual int write(glwebtools::JsonWriter&);
    virtual void Clear();
    virtual int ToJsonString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~ItemPrice();
    void Print();
    unsigned totalSize();
    double GetPrice();
    const char *GetCurrency();
private:
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringValidator,glwebtools::AttributeFormatter> m_currency;  // 0x8
    glwebtools::RequiredArgument<double,oi::AmountValidator,glwebtools::AttributeFormatter> m_price;  // 0x30
};
}

namespace oi {
class ItemPriceArray {  // sizeof 0x18
public:
    virtual int read(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual int read(const glwebtools::JsonReader&);
    virtual int write(glwebtools::JsonWriter&);
    unsigned Size();
    unsigned totalSize();
    const double GetPrice(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    const double GetPrice(unsigned);
    const char *GetCurrency(unsigned);
    void Clear();
    const oi::ItemPrice *operator[](unsigned);
    virtual ~ItemPriceArray();
private:
    const oi::ItemPrice *GetItemPrice(unsigned);
    std::vector<oi::ItemPrice,glwebtools::SAllocator<oi::ItemPrice,4> > m_items;  // 0x4
};
}

#endif
