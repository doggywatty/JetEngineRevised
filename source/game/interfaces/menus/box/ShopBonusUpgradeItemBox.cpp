// source/game/interfaces/menus/box/ShopBonusUpgradeItemBox.cpp
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "ShopBonusUpgradeItemBox.h"
#include "../MenuMgr.h"
#include "../Menu_Shop.h"
#include "game/Player.h"
#include "game/Statistics.h"
#include "game/entities/Bonus.h"
#include "game/entities/BonusSet.h"
#include "game/gamemanagers/BonusUpgradeMgr.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/interfaces/interfaceobjects/Interface3DElement.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/online/store/Price.h"
#include "game/online/store/Store.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include <functional>

bool ShopBonusUpgradeItemBox::checkClickBuyOnShop = false;  // @0xd0d028

// @0x4d8878: the buttons of the layer; a golden item shows its price with the text of the button a bit higher, the others
// open and close their description by touching it; the 3D icon of the bonus, and the item closed
ShopBonusUpgradeItemBox::ShopBonusUpgradeItemBox(Bonus *bonus)
    : BasicPage(jet::String("Shop_Item_Upgrade")), m_bonus(bonus), m_shownLevel(0), m_hasPricePromo(false), m_prevPrice(0), m_hasAmountPromo(false), m_prevAmount(0),
      m_purchaseInProgress(false), m_unused1d9(false), m_collapseButton(0) {
    SetSize(GetUIObject(jet::String("Size"))->GetSize());
    m_buyButton = GetUIButton(jet::String("BuyButton"));
    m_expandButton = GetUIButton(jet::String("ExpandButton"));
    m_buyOffButton = GetUIButton(jet::String("BuyButton_off"));
    m_openButton = GetUIButton(jet::String("OpenButton"));
    if (m_bonus->IsGolden()) {
        Price *next = m_bonus->GetPriceForNextLevel();
        if (next && next->GetPriceType() == Price::IAP) {
            m_buyButton->m_frameTextOff.y *= 0.65f;
            if (next->GetItemDescription().empty()) next->SetItemDescription(m_bonus->GetBonusName());
        }
        EnableExpandedInfo(false);
    } else {
        if (m_bonus->GetGameItemType() == 13) {
            m_expandButton = GetUIButton(jet::String("Item_BlindBox_ExpandButton"));
            m_collapseButton = new InterfaceButton();
            m_collapseButton->InitFrom(GetUIObject(jet::String("Item_BlindBox_CollapseArea")));
        } else {
            m_collapseButton = new InterfaceButton();
            m_collapseButton->InitFrom(GetUIObject(jet::String("Item_Description")));
        }
        m_collapseButton->SetActive(false);
        EnableExpandedInfo(false);
    }
    const clara::DataEntity *deco = m_bonus->GetShopDecoRef();
    const math::vec2<float> pos = GetUIObject(jet::String("Bonus3DPos"))->GetPos();
    m_icon3d = ManualAddI3DElement(this, pos, 1.0f, deco->GetName(), true);
    Refresh();
    EnableExpandedInfo(true);
}

// @0x4d8ecc
ShopBonusUpgradeItemBox::~ShopBonusUpgradeItemBox() { delete m_collapseButton; }

// @0x4d8f60: the name with the level, the price of the next level on the buy button (or, when it is bought to the end, the
// label of the maximum), what the next level gives and the bar of the levels
void ShopBonusUpgradeItemBox::Refresh() {
    jet::String description, label;
    babel::StringMgr *texts = babel::Babel::GetInstance()->GetStringMgr();
    m_shownLevel = m_bonus->GetCurrentLevel();
    Price *next = m_bonus->GetPriceForNextLevel();
    InterfaceText *name = GetUIText(jet::String("Item_Name"));
    int shown = m_bonus->GetLevelsCount() - 1;
    if (shown > m_bonus->GetCurrentLevel() + 1) shown = m_bonus->GetCurrentLevel() + 1;
    bool iapNext = false, busy = false;
    if (next && next->GetPriceType() == Price::IAP) {
        iapNext = true;
        busy = Store::GetInstance()->IsIAPPurchaseInProgress();
    }
    m_purchaseInProgress = busy;
    m_unused1d9 = false;
    m_hasPricePromo = next ? next->HasPricePromotion() : false;
    m_prevPrice = next ? next->GetPriceAsDouble() : 0.0;
    m_hasAmountPromo = next ? next->HasAmountPromotion() : false;
    m_prevAmount = next ? next->GetAmount() : 0;
    if (iapNext) {
        m_itemDescription = next->GetItemDescription();
        name->SetText(m_itemDescription);
    } else {
        Price *current = m_bonus->GetPrice(m_bonus->GetCurrentLevel());
        if (current && current->GetPriceType() == Price::IAP) {
            m_itemDescription = current->GetItemDescription();
            name->SetText(m_itemDescription);
        } else {
            name->SetText(jet::String::Format(m_bonus->GetBonusName().c_str(), shown));
        }
    }
    const bool blindBox = m_bonus->GetGameItemType() == 13;
    const bool hasGoldenBoxes = blindBox ? Player::GetInstance()->CountUnopenedGoldenBlindBoxes() != 0 : false;
    GetUIObject(jet::String("IAPWaitAnim"))->SetVisible(busy);
    bool priceExists = false;
    if (!busy) {
        if (next) {
            m_buyButton->SetVisible(!m_unused1d9 ? !hasGoldenBoxes : false);
            m_buyOffButton->SetVisible(m_unused1d9 ? !hasGoldenBoxes : false);
            m_openButton->SetVisible(hasGoldenBoxes);
            Price *current = m_bonus->GetPrice(m_bonus->GetCurrentLevel());
            Price *shownPrice = current ? current : next;
            int percent = m_bonus->GetPercentUpgradeDisplay(m_bonus->GetCurrentLevel());
            const char *format = m_bonus->GetBonusDescriptionForNextLevel().c_str();
            if (percent <= 0) percent = shownPrice->GetAmount();
            description = jet::String::Format(format, percent);
            if (m_hasPricePromo) {
                GetUIText(jet::String("Price_old"))->SetText(next->GetRegularPriceAsString());
                GetUIText(jet::String("Price_new"))->SetText(Price::FixCurrencySymbol(next->GetPriceAsString()));
                const jet::String key("SHOP_PERCENT_OFF");
                const int off = next->GetPricePercentOff();
                GetUIText(jet::String("Promo_value"))->SetText(jet::String::Format(texts->Get(key).c_str(), off));
                m_buyButton->SetText("");
            } else if (next->GetPriceType() == Price::IAP) {
                InterfaceText *priceIap = GetUIText(jet::String("Price_iap"));
                const jet::String price = next->GetPriceAsString();
                // a long price text is shown whole, not scrolled
                if (!price.empty() && price.length() > 10) {
                    priceIap->SetScale(1.0f);
                    priceIap->EnableAutoScroll(false);
                }
                priceIap->SetText(Price::FixCurrencySymbol(price));
            } else if (m_unused1d9) {
                m_buyOffButton->SetText(Price::FixCurrencySymbol(next->GetPriceAsString()));
            } else {
                m_buyButton->SetText(Price::FixCurrencySymbol(next->GetPriceAsString()));
            }
            GetUIObject(jet::String("Icon_bananas"))->SetVisible(next->GetPriceType() == Price::Bananas);
            GetUIObject(jet::String("Icon_tokens"))->SetVisible(next->GetPriceType() == Price::Tokens);
            priceExists = true;
        } else {
            label = m_bonus->GetBonusLabelMaxUpgrade();
            description = m_bonus->GetBonusDescriptionForNextLevel();
        }
    } else {
        description = m_bonus->GetBonusDescriptionForNextLevel();
    }
    GetUIText(jet::String("Purchased_text"))->SetText(label);
    GetUIObject(jet::String("BuyButton_parent"))->SetVisible(priceExists);
    GetUIObject(jet::String("ButtonPromo"))->SetVisible(m_hasPricePromo);
    const GameItemType type = m_bonus->GetGameItemType();
    const int level = m_bonus->GetCurrentLevel();
    const bool golden = m_bonus->IsGolden();
    GetUIObject(jet::String("GoldenItem_parent"))->SetVisible(golden);
    GetUIObject(jet::String("BlindBoxItem_parent"))->SetVisible(type == 13);
    GetUIObject(jet::String("NormalItem_parent"))->SetVisible(type != 13 && !golden);
    if (golden) {
        GetUIText(jet::String("Item_Golden_Description"))->SetText(description);
    } else if (type == 13) {
        GetUIText(jet::String("Item_BlindBox_Description"))->SetText(description);
    } else {
        GetUIText(jet::String("Locked_text"))->SetVisible(level < 0);
        InterfacePBar *bar = GetUIPBar(jet::String("LevelPBar"));
        bar->SetVisible(!(level < 0));
        bar->SetValue((float)level / (float)(m_bonus->GetLevelsCount() - 1));
        bar->Update(1);
        GetUIText(jet::String("Item_Description"))->SetText(description);
    }
}

// @0x4d9f20: the buttons pressed: the buy one pays the next level (a real-money price is delivered, which is free here),
// the item's own ones open and close its description; and the item refreshes when what it shows is not true any more
void ShopBonusUpgradeItemBox::Update(int dt) {
    BasicPage::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    for (int i = 0; i < mm->ACTION_GetNumReleasedButtons(); i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_buyOffButton) {
            Price *next = m_bonus->GetPriceForNextLevel();
            if (next && next->GetPriceType() == Price::IAP) Store::GetInstance()->BuyItem(next->GetName());
        } else if (b == m_buyButton) {
            Price *next = m_bonus->GetPriceForNextLevel();
            if (!next) break;
            if (next->GetPriceType() == Price::IAP) {
                // the store's tracking of the purchase is not ported
                bool alreadyBought = false;
                BonusUpgradeMgr::GetInstance()->HandleDeliverIAPItem(next, alreadyBought);
            } else {
                if (!Player::GetInstance()->ExecuteCost(*next, std::bind(&ShopBonusUpgradeItemBox::BuyUpgrade, this, std::placeholders::_1, std::placeholders::_2))) break;
                checkClickBuyOnShop = true;
                SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);  // HandleSound @0x4daaa8
                m_bonus->OnCurrentLevelPurchased(0, 0);
                Statistics::GetInstance()->AddShopSpentPrice(*next, true);
                return;
            }
            break;
        } else if (b == m_expandButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            EnableExpandedInfo(true);
            break;
        } else if (b == m_collapseButton) {
            EnableExpandedInfo(false);
            break;
        } else if (b == m_openButton) {
            // PopupMgr::PushPopup(PopupBlindBoxPrize(blind box)): the blind box bonus is not in this build
            break;
        }
    }
    if (!IsRefreshNeeded()) return;
    if (m_shownLevel != m_bonus->GetCurrentLevel()) {
        // the first level of an item that has a golden version sends the shop to its bonus tab; the golden item makes the
        // menu refresh itself
        const bool hasGolden = BonusSet::GetInstance()->GetBonus(m_bonus->GetGameItemType(), true) != 0;
        const bool first = !m_bonus->IsGolden() && m_bonus->GetCurrentLevel() == 0;
        Menu_Base *menu = mm->GetCurrentPlayingMenu();
        const bool inShop = menu && menu->GetMenuName().Equals(Menu_Shop::k_menuName);
        if (first && hasGolden && inShop) {
            static_cast<Menu_Shop*>(menu)->GoToTab(Menu_Shop::Shop_Bonus, true, true);
            return;
        }
        if (m_bonus->IsGolden() && inShop) menu->RefreshOnNextFrame();
    }
    Refresh();
}

// @0x4da350
void ShopBonusUpgradeItemBox::Reset() {
    BasicPage::Reset();
    if (m_collapseButton) m_collapseButton->Reset();
}

// @0x4da34c
void ShopBonusUpgradeItemBox::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) { BasicPage::Render(cam, painter); }

// @0x4da378: a golden item is not expanded; the others show their box or the larger one with the description, and the
// item is as tall as the box says
void ShopBonusUpgradeItemBox::EnableExpandedInfo(bool expand) {
    if (m_bonus->IsGolden()) return;
    if (m_bonus->GetGameItemType() == 13) {
        const bool puzzle = CostumeMgr::GetInstance()->Puzzle_AnyPuzzlePiecesLeft();
        GetUIObject(jet::String("Item_BlindBox_box"))->SetVisible(!expand);
        GetUIObject(jet::String("Item_BlindBox_Expanded"))->SetVisible(expand);
        GetUIObject(jet::String("Item_BlindBox_PrizeInfo_Revive"))->SetVisible(expand && !puzzle);
        GetUIObject(jet::String("Item_BlindBox_PrizeInfo"))->SetVisible(puzzle && expand);
    } else {
        GetUIObject(jet::String("Item_box"))->SetVisible(!expand);
        GetUIObject(jet::String("Item_box_Expand"))->SetVisible(expand);
    }
    SetSize(GetUIObject(jet::String(expand ? "Size_Expand" : "Size"))->GetSize());
    if (m_collapseButton) m_collapseButton->SetActive(expand);
}

// @0x4da80c
void ShopBonusUpgradeItemBox::EnableTouches(bool enable) {
    m_buyButton->SetActive(enable);
    m_expandButton->SetActive(enable);
}

// @0x4da840: what the item shows is not what the bonus says now
bool ShopBonusUpgradeItemBox::IsRefreshNeeded() {
    if (m_shownLevel != m_bonus->GetCurrentLevel()) return true;
    Price *next = m_bonus->GetPriceForNextLevel();
    const bool promo = next ? next->HasPricePromotion() : false;
    const double price = next ? next->GetPriceAsDouble() : 0.0;
    if (m_hasPricePromo != promo || m_prevPrice != price) return true;
    const bool amountPromo = next ? next->HasAmountPromotion() : false;
    const int amount = next ? next->GetAmount() : 0;
    if (m_hasAmountPromo != amountPromo || m_prevAmount != amount) return true;
    const bool busy = next && next->GetPriceType() == Price::IAP ? Store::GetInstance()->IsIAPPurchaseInProgress() : false;
    if (m_purchaseInProgress != busy) return true;
    if (next) return next->GetItemDescription() != m_itemDescription;
    Price *current = m_bonus->GetPrice(m_bonus->GetCurrentLevel());
    return current ? current->GetItemDescription() != m_itemDescription : false;
}

// @0x4da9dc: where the tutorial's arrow points
math::vec2<float> ShopBonusUpgradeItemBox::GetTutorialPos() { return GetUIObject(jet::String("Tutorial_Pos"))->GetCombinedPos(); }

// @0x4daa9c
GameItemType ShopBonusUpgradeItemBox::GetGameItemType() const { return m_bonus->GetGameItemType(); }

// @0x4daad8: the next level is paid and bought
void ShopBonusUpgradeItemBox::BuyUpgrade(int a, int b) {
    Price *next = m_bonus->GetPriceForNextLevel();
    if (!Player::GetInstance()->ExecuteCost(*next)) return;
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);  // HandleSound @0x4daaa8
    m_bonus->OnCurrentLevelPurchased((unsigned)a, (unsigned)b);
    Statistics::GetInstance()->AddShopSpentPrice(*next, true);
}
