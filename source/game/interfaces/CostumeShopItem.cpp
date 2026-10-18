// source/game/interfaces/CostumeShopItem.cpp
//
// Not ported: the popups
// (PopupMgr: the "no internet" and the "download this costume?" questions, PopupPuzzlePieces), the tracking of a
// purchase and the debug logs of the .so. Where a popup asks to go on with the DLC download, the download is started.
#include "CostumeShopItem.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "game/Game.h"
#include "game/Player.h"
#include "game/Statistics.h"
#include "game/entities/MinionCostume.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceArea.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/menus/Menu_MainMenu.h"
#include "game/interfaces/menus/Menu_Shop.h"
#include "game/online/store/Price.h"
#include "game/online/store/Store.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"
#include "system.h"
#include "video/display.h"
#if __has_include("game/interfaces/menus/Menu_BaseCostumes.h")
#include "game/interfaces/menus/Menu_BaseCostumes.h"
#define COSTUMESHOPITEM_BASECOSTUMES 1
#endif
#include <functional>

const int CostumeShopItem::k_offsetDelayTime = 150;
const int CostumeShopItem::k_SkillsExpand_open = 248;
const int CostumeShopItem::k_SkillsExpand_close = 249;
bool CostumeShopItem::sBoughtCustomes = false;

// the downloaded costumes are shown once their archive is mounted: whether that was done, and the updates waited for it
static bool g_bCostumeReloaded = false;
static int g_iDlcDelayCount = 0;

static jet::String Text(const char *key) { return babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(key)); }

// @0x5079c4: the layer of the main menu or of the shop, its elements, the panel in or out of the screen (the main menu
// brings it in itself, the shop has it slide in at once)
CostumeShopItem::CostumeShopItem(bool isMainMenu)
    : BasicPage(jet::String(isMainMenu ? "MainMenu_Shop_Item" : "Shop_Item")), m_isMainMenu(isMainMenu), m_state(0), m_areaOffsetY(0), m_offsetDelayer(0.0f, 0.0f, 1000),
      m_costume(0), m_isDlcRequired(false), m_unused231(false), m_expanded(true), m_expandByTouch(false), m_buyingCostume(0), m_smallBonusText(false) {
    m_width = 0;
    m_unused1F8[0] = m_unused1F8[1] = m_unused1F8[2] = 0;
    m_name = GetUIText(jet::String("Item_name"));
    m_nameBox = GetUIGraph(jet::String("Item_name_box"));
    m_buyButton = GetUIButton(jet::String("BuyButton"));
    m_puzzleButton = GetUIButton(jet::String("PuzzleButton"));
    m_equipButton = GetUIButton(jet::String("EquipButton"));
    m_equippedText = GetUIText(jet::String("Equipped_text"));
    m_downloadBar = 0;
    m_iconBananas = GetUIGraph(jet::String("Icon_bananas"));
    m_iconTokens = GetUIGraph(jet::String("Icon_tokens"));
    m_priceOld = GetUIText(jet::String("Price_old"));
    m_priceNew = GetUIText(jet::String("Price_new"));
    m_priceStroke = GetUIGraph(jet::String("Price_stroke"));
    m_promoTag = GetUIGraph(jet::String("Promo_tag"));
    m_promoValue = GetUIText(jet::String("Promo_value"));
    m_bonus1 = GetUIText(jet::String("Text_Bonus_1"));
    m_bonus2 = GetUIText(jet::String("Text_Bonus_2"));
    m_multiplier = GetUIText(jet::String("Text_Multiplier"));
    m_waitAnim = GetUIGraph(jet::String("Wait_anim"));
    // a 480 pixels wide window in German or Russian main menu has a smaller bonus text
    const math::vec2<unsigned int> window = jet::System::GetDisplay(0)->GetSize();
    const int lang = (int)babel::Babel::GetInstance()->GetSelectedLanguage();
    const bool narrow = window.x == 480 && (window.y == 800 || window.y == 854 || window.y == 840);
    m_smallBonusText = isMainMenu && narrow && (lang == 0x6465 || lang == 0x7275);
    m_area = GetUIArea(jet::String("Costume_UI_Area"));
    const float width = m_area->GetSize().x;
    m_width = width;
    m_offsetDelayer = jet::core::Delayer<float, jet::core::SmootherStep>(width, 0.0f, 1000);
    m_offsetDelayer.SetTargetValue(isMainMenu ? width : 0.0f);
    m_offsetDelayer.SetDelay(k_offsetDelayTime);
    SetState(0);
    m_expandButton = isMainMenu ? GetUIButton(jet::String("ExpandButton")) : 0;
    m_itemButtonBox = isMainMenu ? GetUIGraph(jet::String("Item_button_box")) : 0;
    m_waitAnim->SetVisible(false);
    Expand(false, false);
}

// @0x50949c
CostumeShopItem::~CostumeShopItem() {}

// @0x508fe8: the costume's name and bonuses; one that is not in the data has the download in place of the wear button
// (its bonus text is the place-holder costume's), with a bar over the button once the download is asked for
void CostumeShopItem::SetCostume(MinionCostume *c) {
    if (c == m_costume) return;
    m_costume = c;
    m_name->SetText(c->GetGameName());
    m_isDlcRequired = c->IsShopElement() ? !c->CanLoadModel() : true;
    if (m_smallBonusText && m_bonus1) m_bonus1->SetFixedScale(0.7f);
    if (m_isDlcRequired) {
        Game *game = Game::GetInstance();
        const jet::String &dlc = Game::k_onDemandDlcAssetNames[Game::OnDemandDlc_OptionalCostumes];
        m_bonus1->SetText(CostumeMgr::GetInstance()->GetPlaceHolderCostumeRef()->GetBonusText());
        if (game->IsDlcAvailableOnServer())
            m_equipButton->SetText(jet::String::Format("%s(%.1fM)", Text("CONNECTIVITY_DOWNLOAD_UPDATE").c_str(), (double)((float)game->GetOnDemandDlcSize(dlc) * 9.5367431640625e-07f)));
        else
            m_equipButton->SetText(Text("CONNECTIVITY_DOWNLOAD_UPDATE"));
        m_downloadBar = ManualAddPBar(0, m_equipButton->GetPos(), 0, m_equipButton->GetSpriteIdx(), -1, 0x116, -1, -1, -1, 1);
        m_downloadBar->SetSize(m_equipButton->GetSize());
        m_downloadBar->SetAlpha(0x50);
        m_downloadBar->SetSmoothValue(true);
        m_downloadBar->SetVisible(false);
        if (game->IsOnDemandDlcRequested(dlc) && !game->IsOnDemandDlcReady(dlc)) {
            m_downloadBar->SetValue(game->GetOnDemandDlcProgress(dlc));
            m_downloadBar->SetVisible(true);
            m_equipButton->SetText(Text("CONNECTIVITY_DOWLOADING"));
        }
        m_bonus2->SetVisible(false);
        m_multiplier->SetVisible(false);
    } else {
        m_equipButton->ResetTextToDefault();
        const jet::String &bonus = c->GetBonusText();
        m_bonus2->SetText(bonus);
        m_bonus1->SetText(bonus);
        const int factor = m_costume->GetExtraDespicablePointsFactorForGlobal();
        if (factor > 0) m_multiplier->SetText(jet::String::Format(Text("COSTUME_BONUS_DESPICABLE_MULTIPLIER").c_str(), factor));
        if (m_downloadBar) m_downloadBar->SetVisible(false);
    }
    UpdateButtonsVisibility();
}

// @0x5094ec: a purchase delivered, the panel's slide, the download going on (the bar, and the costumes of the menus read
// again once it is done), and the buttons touched
void CostumeShopItem::Update(int dt) {
    if (sBoughtCustomes) {
        sBoughtCustomes = false;
        OnBuyCostomeSuccess();
    }
    if (m_offsetDelayer.IsUpdating()) {
        m_offsetDelayer.Update(dt);
        m_area->SetOffset(math::vec2<float>(m_offsetDelayer.GetCurrentValue(), m_areaOffsetY));
    }
    if (m_offsetDelayer.GetCurrentValue() >= m_width) return;
    UpdateButtonsVisibility();
    BasicPage::Update(dt);
    if (m_offsetDelayer.GetCurrentValue() != 0 || !IsVisible()) return;

    Game *game = Game::GetInstance();
    const jet::String &dlc = Game::k_onDemandDlcAssetNames[Game::OnDemandDlc_OptionalCostumes];
    if (m_downloadBar && m_downloadBar->IsVisible()) {
        const float progress = game->GetOnDemandDlcProgress(dlc);
        m_downloadBar->SetValue(progress);
        if (game->IsDlcAvailableOnServer()) {
            const float total = (float)game->GetOnDemandDlcSize(dlc);
            m_equipButton->SetText(jet::String::Format("%s(%.1fM/%.1fM)", Text("CONNECTIVITY_DOWLOADING").c_str(), (double)(progress * 9.5367431640625e-07f * total),
                                                       (double)(total * 9.5367431640625e-07f)));
        }
    }
    // the download is done: after 20 updates the menus that show costumes look for the new ones (the loaders' sets of
    // names that failed to load, TextureLoader/ModelLoader::CleanMissingCache @0x6aabac/@0x702b04, do not exist in the
    // ported loaders, which try again)
    if (!g_bCostumeReloaded && game->IsOnDemandDlcRequested(dlc) && game->IsOnDemandDlcReady(dlc) && ++g_iDlcDelayCount > 20) {
        g_bCostumeReloaded = true;
#ifdef COSTUMESHOPITEM_BASECOSTUMES
        Menu_Base *shop = MenuMgr::GetInstance()->GetMenu(Menu_Shop::k_menuName);
        Menu_Base *mainMenu = MenuMgr::GetInstance()->GetMenu(Menu_MainMenu::k_menuName);
        if (shop) static_cast<Menu_BaseCostumes*>(shop)->CheckForNewCostumes();
        if (mainMenu) static_cast<Menu_BaseCostumes*>(mainMenu)->CheckForNewCostumes();
#endif
    }

    MenuMgr *mm = MenuMgr::GetInstance();
    for (int i = 0; i < mm->ACTION_GetNumReleasedButtons(); i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_expandButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            Expand(!IsExpanding(), true);
        }
        if (b == m_buyButton || b == m_puzzleButton) {
            BuyCostume(0, 0);
            return;
        }
        if (b != m_equipButton) continue;
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
        if (!m_isDlcRequired) {
            CostumeMgr::GetInstance()->SetEquippedCostume(m_costume, false);
        } else if (game->AreOnlineServicesReady()) {
            // the .so asks DECRIPTION_CONFIRM_DOWLOAD_DLC (with the size of the download, FormatMeasure of the
            // table's size or of 7.7 when it is not known) in a popup that answers DOWNLOAD_BUTTON or BACK
            downloadDlc();
        }  // else the popup POPUP_NO_INTERNET
        return;
    }
}

// @0x50a2c8: nothing is drawn while the panel is out of the screen
void CostumeShopItem::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    if (!IsVisible()) return;
    if (m_offsetDelayer.GetCurrentValue() >= m_width) return;
    BasicPage::Render(cam, painter);
}

// @0x509dd4: which buttons and texts show for the costume: the price (with a promotion's old price and percent) to buy
// one that can be, the pieces of a puzzle, to wear one that is owned and not worn, "equipped" for the worn one and the
// multiplier of the despicable points for one not owned
void CostumeShopItem::UpdateButtonsVisibility() {
    const bool equipped = m_costume->IsCurrentlyEquipped();
    CostumeMgr *costumes = CostumeMgr::GetInstance();
    bool canBuy = false, owned = false, puzzle = false;
    if (costumes->WasBought(*m_costume) || equipped) {
        owned = true;
    } else if (!m_isDlcRequired) {
        canBuy = m_costume->GetBuyType().Equals(MinionCostume::k_buyType_buyable);
        puzzle = m_costume->GetBuyType().Equals(MinionCostume::k_buyType_puzzle);
    }
    if (canBuy) {
        Price *price = m_costume->GetPrice();
        const bool promo = price->HasPricePromotion() && !owned && !m_isDlcRequired;
        const bool tokens = price->GetPriceType() == Price::Tokens;
        const bool iap = price->GetPriceType() == Price::IAP;
        // the price text of a purchase is moved in the button (m_fromAurora marks that it is in its place)
        if (iap) {
            if (m_buyButton->m_fromAurora) {
                m_buyButton->m_frameTextOff.x *= m_isMainMenu ? 0.699999988f : 0.600000024f;
                m_buyButton->m_fromAurora = false;
            }
        } else if (!m_buyButton->m_fromAurora) {
            m_buyButton->m_frameTextOff.x *= m_isMainMenu ? 1.42857146f : 1.66666675f;
            m_buyButton->m_fromAurora = true;
        }
        m_iconBananas->SetVisible(!iap && !tokens);
        m_iconTokens->SetVisible(tokens);
        m_priceStroke->SetVisible(promo);
        m_promoTag->SetVisible(promo);
        m_priceOld->SetVisible(promo);
        m_priceNew->SetVisible(promo);
        m_promoValue->SetVisible(promo);
        if (!promo) {
            m_buyButton->SetText(price->GetPriceAsString());
        } else {
            m_buyButton->SetText(jet::String(""));
            m_priceOld->SetText(price->GetRegularPriceAsString());
            m_priceNew->SetText(price->GetPriceAsString());
            m_promoValue->SetText(jet::String::Format(Text("SHOP_PERCENT_OFF").c_str(), price->GetPricePercentOff()));
        }
    }
    if (puzzle)
        m_puzzleButton->SetText(jet::String::Format("%d / %d", costumes->Puzzle_GetPiecesOwnedCountFor(*m_costume), m_costume->GetPuzzlePiecesCount()));
    // a purchase of this costume going on shows the wait animation in place of the buy button
    bool buying = false;
    const Price *price = m_costume->GetPrice();
    if (price->GetPriceType() == Price::IAP) {
        Store *store = Store::GetInstance();
        if (store->IsIAPPurchaseInProgress()) buying = store->GetIAPItemBeingBought().Equals(price->GetItemName());
    }
    m_buyButton->SetVisible(canBuy && !buying);
    m_puzzleButton->SetVisible(puzzle);
    m_equipButton->SetVisible(owned && !equipped ? true : m_isDlcRequired);
    m_equippedText->SetVisible(owned && equipped && !m_isDlcRequired);
    const bool multiplier = m_costume->GetExtraDespicablePointsFactorForGlobal() != 0 && !m_isDlcRequired && !owned;
    m_multiplier->SetVisible(multiplier);
    m_bonus2->SetVisible(multiplier);
    m_bonus1->SetVisible(!multiplier);
    m_waitAnim->SetVisible(buying);
}

// @0x50a314
void CostumeShopItem::SetState(int s) { m_state = s; }

// @0x50a31c
void CostumeShopItem::SetButtonsActive(bool active) {
    m_buyButton->SetActive(active);
    m_equipButton->SetActive(active);
}

// @0x50a350
void CostumeShopItem::SetTransition(bool inside) { m_offsetDelayer.SetTargetValue(inside ? 0.0f : m_width); }

// @0x50a378: a costume of bananas or tokens is paid and worn at once; one of real money is bought in the store (the
// ones the .so knows, here the purchase is free and is delivered at once, as Price::DeliverIAPItem would on a purchase
// that went through); a puzzle opens the popup of its pieces
void CostumeShopItem::BuyCostume(int, int) {
    MinionCostume *costume = m_costume;
    if (costume->GetBuyType().Equals(MinionCostume::k_buyType_buyable)) {
        Price *price = costume->GetPrice();
        const int id = costume->GetCostumeId();
        if (price->GetPriceType() == Price::IAP) {
            m_buyingCostume = costume;
            Store *store = Store::GetInstance();
            if (store->IsAnyIAPItemAvailable()) {  // else the popup STORE_NO_PACKS_AVAILABLE
                if (id == 0x19 || id == 0xf || id == 0x18 || id == 2 || (id & ~2) == 5) {
                    store->BuyItem(price->GetItemName());
                    sBoughtCustomes = true;
                }
            }
        } else {
            SoundMgr::GetInstance()->StopGroup(SoundMgr::k_sfxGroupLabel, 500);  // the .so logs "pause sound here" first
            if (Player::GetInstance()->ExecuteCost(*price, std::bind(&CostumeShopItem::BuyCostume, this, std::placeholders::_1, std::placeholders::_2))) {
                HandleSound();
                CostumeMgr::GetInstance()->MarkAsBought(*costume);
                Statistics::GetInstance()->AddShopSpentPrice(*price, true);
                CostumeMgr::GetInstance()->SetEquippedCostume(costume, false);
            }
        }
    } else if (costume->GetBuyType().Equals(MinionCostume::k_buyType_puzzle)) {
        SoundMgr::GetInstance()->StopGroup(SoundMgr::k_sfxGroupLabel, 500);
        // then PopupPuzzlePieces(costume, false) in PopupMgr
    }
}

// @0x50a75c: the costume of the purchase is owned and worn
void CostumeShopItem::OnBuyCostomeSuccess() {
    // ev_sfx_buy, when it is not playing
    CostumeMgr::GetInstance()->MarkAsBought(*m_buyingCostume);
    CostumeMgr::GetInstance()->SetEquippedCostume(m_buyingCostume, false);
}

// @0x50a7c0
void CostumeShopItem::HandleSound() { SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0); }

// @0x50a7f0: the answer yes of the question to download the costumes
void CostumeShopItem::ConfirmDownloadDlc(void *item) { static_cast<CostumeShopItem*>(item)->downloadDlc(); }

// @0x50a7f4: the download of the optional costumes is asked for, the bar and the text show it going on; without the
// table of contents from the server there is no download (the popup POPUP_NO_INTERNET). The .so also lets Game show
// the next DLC error at once (Game+0x26c, the countdown of Game::UpdateDlc, not ported)
void CostumeShopItem::downloadDlc() {
    Game *game = Game::GetInstance();
    m_downloadBar->SetVisible(true);
    m_equipButton->SetText(Text("CONNECTIVITY_DOWLOADING"));
    g_bCostumeReloaded = false;
    if (game->IsDlcAvailableOnServer()) {
        g_iDlcDelayCount = 0;
        game->DownloadOnDemandDlc(Game::k_onDemandDlcAssetNames[Game::OnDemandDlc_OptionalCostumes], m_isMainMenu ? Game::DlcTracking_MainMenu : Game::DlcTracking_Shop);
    }
}

// @0x50a99c
bool CostumeShopItem::IsExpanding() const { return m_expanded; }

// @0x50a9a4: the bonuses of the main menu's panel open (the name box shows, the button is up in place and turns to close)
// or fold (it goes down to where the name box is); once a touch did it only a touch does
void CostumeShopItem::Expand(bool expand, bool byTouch) {
    if (!m_expandButton) return;
    if (m_expandByTouch && !byTouch) return;
    m_expanded = expand;
    m_expandByTouch = byTouch;
    m_nameBox->SetVisible(expand);
    if (expand) {
        m_expandButton->SetOffset(math::vec2<float>(0, 0));
        m_expandButton->SetIdleAnim(k_SkillsExpand_close);
        m_expandButton->SetPressedAnim(k_SkillsExpand_open);
    } else {
        const float down = m_itemButtonBox->GetPos().y - m_nameBox->GetPos().y;
        m_expandButton->SetOffset(math::vec2<float>(0, down));
        m_expandButton->SetIdleAnim(k_SkillsExpand_open);
        m_expandButton->SetPressedAnim(k_SkillsExpand_close);
    }
}
