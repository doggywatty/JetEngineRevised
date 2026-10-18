// libs/offlineitems/include/offlineitems/object/oi_storeitem.h
#ifndef LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_STOREITEM_H
#define LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OBJECT_OI_STOREITEM_H

namespace oi {
class StoreOfflineItem {  // sizeof 0x74
public:
    virtual int read(const glwebtools::JsonReader&);
    virtual int write(glwebtools::JsonWriter&);
    void Clear();
    int ToJsonString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~StoreOfflineItem();
    unsigned totalSize();
    const char *GetName();
    double GetAmount();
    double GetRegularAmount();
    bool HasAmountPromotion();
    double GetAmountReductionPercent();
    const oi::ItemPriceArray *GetPrices(const oi::BillingMethod*);
    const oi::ItemPriceArray *GetRegularPrices(const oi::BillingMethod*);
    bool HasPricePromotion(const oi::BillingMethod*);
    double GetPriceReductionPercent(const oi::BillingMethod*);
    unsigned GetCategoryCount();
    const char *GetCategory(unsigned);
    unsigned GetBillingMethodCount();
    const oi::BillingMethod *GetBillingMethod(const char*, const char*);
    const oi::BillingMethod *GetBillingMethod(unsigned);
    void Print();
protected:
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::StringValidator,glwebtools::AttributeFormatter> m_name;  // 0x4
    glwebtools::RequiredArgument<int,oi::AmountValidator,glwebtools::AttributeFormatter> m_quantity;  // 0x2C
    glwebtools::OptionalArgument<int,oi::AmountValidator,glwebtools::AttributeFormatter> m_regular_quantity;  // 0x38
    unsigned categoryCount;  // 0x44
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::SAllocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,4> > m_category;  // 0x48
    oi::BillingMethodArray m_billingMethodArray;  // 0x5C
};
}

namespace oi {
class StoreOfflineItemArray {  // sizeof 0x18
public:
    StoreOfflineItemArray();
    virtual int read(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual int read(const glwebtools::JsonReader&);
    virtual unsigned GetItemCount();
    virtual const oi::StoreOfflineItem *GetItem(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    virtual const oi::StoreOfflineItem *GetItem(unsigned);
    virtual void Clear();
    virtual const oi::StoreOfflineItem *operator[](std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    virtual const oi::StoreOfflineItem *operator[](unsigned);
    virtual ~StoreOfflineItemArray();
    virtual void Print();
    unsigned totalSize();
protected:
    std::vector<oi::StoreOfflineItem,glwebtools::SAllocator<oi::StoreOfflineItem,4> > offlineItems_array;  // 0x4
};
}

#endif
