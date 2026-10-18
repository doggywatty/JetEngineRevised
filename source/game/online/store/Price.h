// source/game/online/store/Price.h
#ifndef SOURCE_GAME_ONLINE_STORE_PRICE_H
#define SOURCE_GAME_ONLINE_STORE_PRICE_H

#include <vector>
#include "entity.h"
#include "core/protectedstorage.h"
#include "core/pstring.h"

// What an item costs ("Price" data entity, 0x134 bytes in the .so): bananas, tokens, or real money (an in-app
// purchase). The name of the entity is the item's name. Values are kept in ProtectedStorage like the original.
// Real money purchases are not available here: an in-app price reads "FREE" (Price::k_freeText).
class Price : public clara::Entity {
public:
    enum Type { Bananas = 0, Tokens = 1, IAP = 2 };

    explicit Price(const clara::Template*);
    virtual ~Price();
    virtual void PostInit();
    virtual void Reset();  // @0x4092d4: the data's values
    void SetPriceType(Type t) { m_priceType = t; }
    void SetPrice(double);
    void SetRegularPrice(double);
    void SetAmount(int);
    void SetRegularAmount(int v) { m_regularAmount.SetValue(v); }
    void SetItemName(const jet::String &n) { SetName(n); }
    void SetItemId(int i) { m_itemId = i; }
    void SetItemDescription(const jet::String &d) { m_itemDescription = d; }
    void SetActive(bool);
    void SetPriceString(const jet::String &s);
    void SetRegularPriceString(const jet::String &s) { m_regularPriceString = s; }
    void SetCurrencyCode(const jet::String &c) { m_currencyCode = c; }
    void SetPackId(const jet::String &p) { m_packId = p; }
    void SetTrackingUid(const jet::String &u) { m_trackingUid = u; }
    void AddExtraCategory(const jet::String &c) { m_extraCategories.push_back(c); }
    jet::String GetItemName() const { return GetName(); }
    jet::String GetItemDescription() const { return m_itemDescription; }
    jet::String GetCategory() const { return m_category; }
    int GetExtraCategoryCount() const { return (int)m_extraCategories.size(); }
    jet::String GetExtraCategory(int i) const { return i < (int)m_extraCategories.size() ? m_extraCategories[(size_t)i] : jet::String(); }
    int GetItemId() const { return m_itemId; }
    double GetPriceAsDouble() const { return m_price.GetValue(); }
    double GetRegularPriceAsDouble() const { return m_regularPrice.GetValue(); }
    jet::String GetPriceAsString() const;
    jet::String GetRegularPriceAsString() const;
    bool HasPricePromotion() const;
    Type GetPriceType() const { return m_priceType; }
    jet::String GetCurrencyCode() const { return m_currencyCode; }
    jet::String GetPackId() const { return m_packId; }
    jet::String GetTrackingUid() const { return m_trackingUid; }
    int GetAmount() const { return m_amount.GetValue(); }
    int GetRegularAmount() const { return m_regularAmount.GetValue(); }
    bool HasAmountPromotion() const;
    int GetPricePercentOff() const;
    bool IsActive() const { return m_isActive; }
    void BuyItem();

    // @0x408b68: the store's real-money text with its currency code (USD, EUR, ...) replaced by the symbol. The prices
    // this build shows are measures of bananas and tokens or FREE, which have no code, so the text stays as it is
    static jet::String FixCurrencySymbol(const jet::String &s) { return s; }
    static const jet::String k_tmplName, k_freeText;
private:
    jet::String m_category;                      // 0x54
    std::vector<jet::String> m_extraCategories;  // 0x58
    int m_itemId;                                // 0x64
    jet::String m_itemDescription;               // 0x68
    jet::core::ProtectedStorage<double> m_price, m_regularPrice;  // 0x70, 0x98
    Type m_priceType;                            // 0xC0
    jet::core::ProtectedStorage<int> m_amount, m_regularAmount;   // 0xC4, 0xEC
    jet::String m_priceString;                   // 0x114
    jet::String m_regularPriceString;            // 0x118
    jet::String m_currencyCode;                  // 0x11C
    jet::String m_packId;                        // 0x120
    jet::String m_trackingUid;                   // 0x124
    bool m_isActive;                             // 0x128
    static const jet::String k_paramCategory, k_paramValue, k_paramObjectIdTracking, k_paramCurrency, k_paramAmount, k_paramAvailable;
};

#endif
