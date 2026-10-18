// source/game/interfaces/menus/box/ShopBuyableItemBox.cpp
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "game/interfaces/popups/BasePopup.h"
#include "ShopBuyableItemBox.h"
#include "../MenuMgr.h"
#include "game/Game.h"
#include "game/Player.h"
#include "game/entities/Buyable.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceArea.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/online/store/Price.h"
#include "game/online/store/Store.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"
#include "sprites/sprite.h"
#include "sprites/spritemgr.h"
#include "sprites/spriteplayer.h"
#include "utils/painterclipscope.h"
#include <cmath>

// the promotions' layouts show where the layout of the game allows them (COPPAMgr::CanShowPromoLayouts @0x5462fc: it does,
// the manager of the kids' privacy rules is not in this build)
static const bool k_canShowPromoLayouts = true;

// @0x4db4b8
ShopBuyableItemBox::ShopBuyableItemBox(Buyable *item, int) : BasicPage(jet::String("Shop_IAP")), m_buyableItem(item), m_buyButton(0), m_offButton(0), m_waitIcon(0), m_hasPricePromo(false), m_hasAmountPromo(false), m_prevAmount(0), m_prevPrice(0) {
    SetSize(GetUIObject(jet::String("Size"))->GetSize());
    m_bg = GetUIObject(jet::String("Item_box"));
    m_iconArea = GetUIArea(jet::String("Icon_Area"));
    m_itemPrice = item->GetPrice();
    m_name = GetUIText(jet::String("Item_Name"));
    if (m_itemPrice->GetCategory().Equals("Tokens")) {
        m_buyButton = GetUIButton(jet::String("BuyButton_money"));
        GetUIButton(jet::String("BuyButton_token"))->SetVisible(false);
    } else {
        m_buyButton = GetUIButton(jet::String("BuyButton_token"));
        GetUIButton(jet::String("BuyButton_money"))->SetVisible(false);
    }
    m_offButton = GetUIButton(jet::String("BuyButton_off"));
    m_name->SetText(m_itemPrice->GetItemDescription());
    m_waitIcon = GetUIGraph(jet::String("Wait_anim"));
    m_waitIcon->SetVisible(false);
    m_promo = GetUIObject(jet::String("ButtonPromo"));
    UpdateDisplay();
}

// @0x4db9f0
ShopBuyableItemBox::~ShopBuyableItemBox() {}

// @0x4dba48: a pressed buy button pays: the pack of tokens is given (the purchase of the real game is free here), the
// pack of bananas costs its price
void ShopBuyableItemBox::Update(int dt) {
    BasicPage::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    for (int i = 0; i < mm->ACTION_GetNumReleasedButtons(); i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_offButton) {
            Store::GetInstance()->BuyItem(m_buyableItem->GetPrice()->GetName());
        } else if (b == m_buyButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_buy, 0);
            if (m_itemPrice->GetCategory().Equals("Tokens")) {
                SoundMgr::GetInstance()->Play(BasePopup::k_sfx_menu_message_window_pop_up, 0);
                // the store is told where the purchase came from (Store::SetTrackingPurchase*)
                Player::GetInstance()->AddTokens(m_itemPrice->GetAmount());
                Game::GetInstance()->Save(false, false);
            } else if (m_itemPrice->GetCategory().Equals("Bananas")) {
                BuyBannanasPack(0, 0);
            }
            break;
        }
    }
    if (HasPriceChanged()) UpdateDisplay();
    m_waitIcon->SetVisible(false);
    m_buyButton->SetActive(true);
    m_buyButton->SetVisible(true);
    m_offButton->SetVisible(false);
    m_offButton->SetActive(!m_buyButton->IsActive());
    m_promo->SetVisible(true);
}

// @0x4dbcdc: the background, the icon of the pack in the rectangle of its area (clipped to the list), then the rest
void ShopBuyableItemBox::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    m_bg->Render(cam, painter);
    {
        PainterClipScope clip(&painter, m_parent->GetClip(), false);
        const math::vec2<float> pos = m_iconArea->GetPos(), off = GetOffset(), size = m_iconArea->GetSize();
        const math::rect<float> r(off.x + pos.x, off.y + pos.y, off.x + pos.x + size.x, off.y + pos.y + size.y);
        SpritePlayer *icon = m_buyableItem->GetSpritePlayer();
        icon->PutIntoRect(r, 1.0f);
        icon->Render(painter);
    }
    m_bg->SetVisible(false);
    BasicPage::Render(cam, painter);
    m_bg->SetVisible(true);
}

// @0x4dbe68: what is shown is not what the price says now
bool ShopBuyableItemBox::HasPriceChanged() {
    if (m_hasAmountPromo != m_itemPrice->HasAmountPromotion()) return true;
    if (m_hasPricePromo != m_itemPrice->HasPricePromotion()) return true;
    if (m_itemPrice->GetPriceAsDouble() != m_prevPrice) return true;
    if (m_itemPrice->GetAmount() != m_prevAmount) return true;
    if (m_itemPrice->GetPriceType() != Price::IAP) return false;
    return !m_itemPrice->GetItemDescription().Equals(m_itemDescription);
}

// @0x4dbf18: the pack of bananas is paid with its price (tokens), and given
void ShopBuyableItemBox::BuyBannanasPack(int, int) {
    Player *player = Player::GetInstance();
    if (!player->ExecuteCost(*m_itemPrice, std::bind(&ShopBuyableItemBox::BuyBannanasPack, this, std::placeholders::_1, std::placeholders::_2))) return;
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);  // and the rate game event (not ported)
    player->AddBananas(m_itemPrice->GetAmount());
}

// @0x4dc100: its name, the quantity it gives and the price (on its button), with the old values and the tag when it is on sale
void ShopBuyableItemBox::UpdateDisplay() {
    InterfaceText *quantity = GetUIText(jet::String("Item_Quantity"));
    InterfaceText *quantityOld = GetUIText(jet::String("Quantity_old"));
    InterfaceText *quantityNew = GetUIText(jet::String("Quantity_new"));
    InterfaceText *priceOld = GetUIText(jet::String("Price_old"));
    InterfaceText *priceNew = GetUIText(jet::String("Price_new"));
    InterfaceObject *promoTag = GetUIGraph(jet::String("Promo_tag"));
    InterfaceText *promoValue = GetUIText(jet::String("Promo_value"));
    InterfaceObject *iapPromotion = GetUIGraph(jet::String("IAP_promotion"));
    InterfaceText *iapPromoValue = GetUIText(jet::String("IAP_Promo_value"));
    babel::Formatter *fmt = babel::Babel::GetInstance()->GetFormatter();
    babel::StringMgr *texts = babel::Babel::GetInstance()->GetStringMgr();
    if (m_itemPrice->GetPriceType() == Price::IAP) {
        m_name->SetText(m_itemPrice->GetItemDescription());
        m_itemDescription = m_itemPrice->GetItemDescription();
    } else {
        m_name->SetText(m_buyableItem->GetBuyableName());
        m_itemDescription = m_buyableItem->GetBuyableName();
    }
    if (m_itemPrice) {
        m_hasAmountPromo = m_itemPrice->HasAmountPromotion();
        m_hasPricePromo = m_itemPrice->HasPricePromotion();
        const bool promo = m_hasPricePromo || m_hasAmountPromo;
        Store *store = Store::GetInstance();
        if (store->HasIAPPromotion()) {
            iapPromotion->SetVisible(false);
            promoTag->SetVisible(k_canShowPromoLayouts && promo);
        } else {
            iapPromotion->SetVisible(k_canShowPromoLayouts && promo);
            promoTag->SetVisible(false);
        }
        quantity->SetVisible(!m_hasAmountPromo);
        quantityOld->SetVisible(k_canShowPromoLayouts && m_hasAmountPromo);
        quantityNew->SetVisible(m_hasAmountPromo);
        priceOld->SetVisible(k_canShowPromoLayouts && m_hasPricePromo);
        priceNew->SetVisible(m_itemPrice->GetPriceType() == Price::IAP ? true : m_hasPricePromo);
        m_prevAmount = m_itemPrice->GetAmount();
        m_prevPrice = m_itemPrice->GetPriceAsDouble();
        if (!promo) {
            if (m_itemPrice->GetPriceType() != Price::IAP) {
                m_buyButton->SetText(Price::FixCurrencySymbol(m_itemPrice->GetPriceAsString()));
                m_offButton->SetText(Price::FixCurrencySymbol(m_itemPrice->GetPriceAsString()));
            } else {
                const jet::String price = m_itemPrice->GetPriceAsString();
                // a long price text is shown whole, not scrolled
                if (!price.empty() && price.length() > 10) {
                    priceNew->SetScale(1.0f);
                    priceNew->EnableAutoScroll(false);
                }
                priceNew->SetText(Price::FixCurrencySymbol(price));
            }
            quantity->SetText(fmt->FormatMeasure(m_itemPrice->GetAmount()));
        } else {
            if (m_hasAmountPromo) {
                quantityOld->SetText(fmt->FormatMeasure(m_itemPrice->GetRegularAmount()));
                quantityNew->SetText(fmt->FormatMeasure(m_itemPrice->GetAmount()));
            } else {
                quantity->SetText(fmt->FormatMeasure(m_itemPrice->GetAmount()));
            }
            jet::String key;
            int percent;
            if (m_hasPricePromo) {
                m_buyButton->SetText("");
                m_offButton->SetText("");
                priceOld->SetText(m_itemPrice->GetRegularPriceAsString());
                priceNew->SetText(Price::FixCurrencySymbol(m_itemPrice->GetPriceAsString()));
                key = jet::String("SHOP_PERCENT_OFF");
                percent = m_itemPrice->GetPricePercentOff();
            } else {
                key = jet::String("SHOP_PERCENT_FREE");
                const float ratio = (float)m_itemPrice->GetAmount() / (float)m_itemPrice->GetRegularAmount();
                percent = (int)std::floor(ratio * 100.0f - 100.0f + 0.5f);
                m_buyButton->SetText(Price::FixCurrencySymbol(m_itemPrice->GetPriceAsString()));
                m_offButton->SetText(Price::FixCurrencySymbol(m_itemPrice->GetPriceAsString()));
            }
            if (store->HasIAPPromotion()) promoValue->SetText(jet::String::Format(texts->Get(key).c_str(), percent));
            else iapPromoValue->SetText(jet::String::Format("-%d%%", percent));
        }
        GetUIObject(jet::String("IAP_Hot_tag"))->SetVisible(false);
        GetUIObject(jet::String("IAP_Hot2_tag"))->SetVisible(false);
        GetUIObject(jet::String("IAP_Banner_tag"))->SetVisible(false);
        // the tokens pack that comes with a costume says so, while the costume is not owned
        CostumeMgr *costumes = CostumeMgr::GetInstance();
        if (!costumes->WasBought(*costumes->GetCostumeRef(0)) && m_itemPrice->GetCategory().Equals("Tokens") && m_itemPrice->GetItemName().Equals("Truckload_of_Tokens"))
            GetUIText(jet::String("Item_Gift_Promo"))->SetText(texts->Get(jet::String("ADDITON_BABY_COSTUME")));
        GetUIObject(jet::String("Icon_tokens"))->SetVisible(m_itemPrice->GetPriceType() == Price::Tokens);
    }
    GetUIObject(jet::String("IAP_tag"))->SetVisible(false);
}
