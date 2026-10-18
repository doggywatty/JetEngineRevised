// source/game/online/store/Price.cpp
#include "Price.h"
#include "Store.h"
#include "game/Game.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include <cmath>

const jet::String Price::k_tmplName("PriceTemplate");  // @0xc33ed2
const jet::String Price::k_freeText("FREE");
const jet::String Price::k_paramCategory("Category");
const jet::String Price::k_paramValue("Value");
const jet::String Price::k_paramObjectIdTracking("ObjectIdTracking");
const jet::String Price::k_paramCurrency("Currency");
const jet::String Price::k_paramAmount("Amount");
const jet::String Price::k_paramAvailable("Available");

// @0x408720
Price::Price(const clara::Template *t) : clara::Entity(t), m_itemId(0), m_price(0.0), m_regularPrice(0.0), m_priceType(Bananas), m_amount(0), m_regularAmount(0), m_isActive(false) {}

// @0x40882c
Price::~Price() {}

// @0x4088e4: the item goes in the store's database of its kind (in-app items or dynamic ones)
void Price::PostInit() {
    clara::Entity::PostInit();
    Reset();
    if (Store *s = Store::GetInstance()) {
        if (m_priceType == IAP) s->AddIAPPriceToDB(this);
        else s->AddDynamicPriceToDB(this);
    }
}

// @0x4092d4
void Price::Reset() {
    m_extraCategories.clear();
    m_itemId = 0;
    GetParam(k_paramCategory, m_category);
    float v = 0;
    GetParam(k_paramValue, v);
    SetPrice((double)v);
    int c = 0;
    GetParam(k_paramCurrency, c);
    m_priceType = (Type)c;
    int amount = 0;
    GetParam(k_paramAmount, amount);
    m_amount.SetValue(amount);
    SetRegularPrice(GetPriceAsDouble());
    m_regularAmount = m_amount;
    GetParam(k_paramAvailable, m_isActive);
}

// @0x408948: a price in bananas or tokens shows as its measure; the regular price follows it until one is set
void Price::SetPrice(double v) {
    m_price.SetValue(v);
    if (m_priceType == IAP || !m_priceString.empty()) {
        if (!(std::fabs(GetRegularPriceAsDouble()) > 0.0)) SetRegularPrice(GetPriceAsDouble());
        return;
    }
    m_priceString = babel::Babel::GetInstance()->GetFormatter()->FormatMeasure((float)v, 2);
    m_priceString = jet::String("$") + m_priceString;
    if (!(std::fabs(GetRegularPriceAsDouble()) > 0.0)) SetRegularPrice(GetPriceAsDouble());
}

// @0x408a50
void Price::SetRegularPrice(double v) {
    m_regularPrice.SetValue(v);
    if (m_priceType == IAP || !m_regularPriceString.empty()) return;
    m_regularPriceString = jet::String("$") + babel::Babel::GetInstance()->GetFormatter()->FormatMeasure((float)v, 2);
}

// @0x409000
void Price::SetAmount(int v) {
    m_amount.SetValue(v);
    if (GetRegularAmount() == 0) m_regularAmount.SetValue(v);
}

// @0x409050
void Price::SetActive(bool a) { m_isActive = a; }

// @0x408b24: the store's text for the price (in-app prices are not offered, see GetPriceAsString)
void Price::SetPriceString(const jet::String &s) {
    m_priceString = s;
    if (m_regularPriceString.empty()) m_regularPriceString = m_priceString;
}

// @0x409400: bananas and tokens as a measure; an in-app price is FREE
jet::String Price::GetPriceAsString() const {
    if (m_priceType == IAP) return k_freeText;
    return babel::Babel::GetInstance()->GetFormatter()->FormatMeasure((float)GetPriceAsDouble(), 2);
}

// @0x409484
jet::String Price::GetRegularPriceAsString() const {
    if (m_priceType == IAP) return k_freeText;
    return babel::Babel::GetInstance()->GetFormatter()->FormatMeasure((float)GetRegularPriceAsDouble(), 2);
}

// @0x40915c: the regular price is more than the price, while the game has promotions on
bool Price::HasPricePromotion() const {
    if (!Game::GetInstance()->ArePricePromotionsOn()) return false;
    return GetRegularPriceAsDouble() > GetPriceAsDouble();
}

// @0x4091dc
bool Price::HasAmountPromotion() const {
    if (!Game::GetInstance()->ArePricePromotionsOn()) return false;
    return GetRegularAmount() < GetAmount();
}

// @0x409830
int Price::GetPricePercentOff() const {
    if (!(std::fabs(GetRegularPriceAsDouble()) > 1e-6)) return 0;
    return (int)std::floor(0.5 + (1.0 - GetPriceAsDouble() / GetRegularPriceAsDouble()) * 100.0);
}

// @0x409280
void Price::BuyItem() { if (Store *s = Store::GetInstance()) s->BuyItem(GetItemName()); }
