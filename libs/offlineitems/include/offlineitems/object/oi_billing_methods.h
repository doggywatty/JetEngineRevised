// libs/offlineitems/include/offlineitems/object/oi_billing_methods.h
#ifndef LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_BILLING_METHODS_H
#define LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_BILLING_METHODS_H

namespace oi {
class BillingMethod {  // sizeof 0x84
public:
    virtual int read(const glwebtools::JsonReader&);
    virtual int write(glwebtools::JsonWriter&);
    virtual void Clear();
    virtual int ToJsonString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~BillingMethod();
    const char *GetName();
    const char *GetType();
    const oi::ItemPriceArray *GetItemPrices();
    const oi::ItemPriceArray *GetItemRegularPrices();
    void Print();
    unsigned totalSize();
protected:
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringValidator,glwebtools::AttributeFormatter> m_name;  // 0x4
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringValidator,glwebtools::AttributeFormatter> m_type;  // 0x2C
    oi::ItemPriceArray m_itemPrices;  // 0x54
    oi::ItemPriceArray m_regular_itemPrices;  // 0x6C
};
}

namespace oi {
class BillingMethodArray {  // sizeof 0x18
public:
    virtual int read(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual int read(const glwebtools::JsonReader&);
    virtual int write(glwebtools::JsonWriter&);
    unsigned Size();
    const oi::BillingMethod *GetBillingMethod(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    const oi::BillingMethod *GetBillingMethod(unsigned);
    void Clear();
    const oi::BillingMethod *operator[](unsigned);
    virtual ~BillingMethodArray();
    unsigned totalSize();
private:
    std::vector<oi::BillingMethod,glwebtools::SAllocator<oi::BillingMethod,4> > m_array;  // 0x4
};
}

#endif
