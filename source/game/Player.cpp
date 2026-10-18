// source/game/Player.cpp
#include "Player.h"
#include "Game.h"
#include "game/online/store/Price.h"
#include "game/SortPageHelp.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/gameenums.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/popups/PopupGoToLocation.h"

const jet::String Player::k_saveRecordName("Player");

Player::Player() { ResetData(m_data); }
Player::~Player() {}

// ResetData @0x51c57c
void Player::ResetData(PlayerSaveData &d) const {
    d.startTokensGiven.SetValue(0);
    d.bananas.SetValue(0);
    d.tokens.SetValue(0);
    d.shortA.SetValue(0);
    d.shortB.SetValue(0);
    d.intF0.SetValue(0);
    d.int118.SetValue(0);
    d.int140.SetValue(0);
    d.int168.SetValue(0);
    d.flag.SetValue(0);
    d.locationsUnlocked.SetValue(1);
    d.unopenedSilverBoxes.clear();
    d.goldenBoxes.SetValue(0);
    d.firstMissionComplete.SetValue(0);
    d.meenaRamTutoDisplayed.SetValue(0);
    d.reviveTutoComplete.SetValue(0);
    d.firstPressPageButton.SetValue(1);
    d.dailyCurTime.SetValue(0);
    d.dailyDays.SetValue(0);
    d.getFirstCostume.SetValue(0);
    d.getSecondCostume.SetValue(0);
    d.awardCostumeTime1.SetValue(0);
    d.awardCostumeTime2.SetValue(0);
    d.magicCostumeTime.SetValue(0);
    d.isMagicCostume.SetValue(0);
    d.sideLookTutorialDone.SetValue(0);
    d.lastLuckyDrawDay.SetValue(0);
    d.firstLoginDay.SetValue(0);
    d.notFirstDraw.SetValue(0);
    d.firstBuy.SetValue(0);
    d.redeemCostumes.clear();
    d.bossTutorials.SetValue(0);
}

void Player::SaveDataInStream(jet::stream::IStreamW &s, PlayerSaveData &d) const {
    int version = k_saveGameVersion;
    s << version;
    int sig = k_signatureV5;
    s << sig;
    int i;
    short sh;
    char c;
#define PI(f) i = d.f.GetValue(); s << i
    PI(startTokensGiven); PI(bananas); PI(tokens);
    sh = d.shortA.GetValue(); s << sh;
    sh = d.shortB.GetValue(); s << sh;
    PI(intF0); PI(int118); PI(int140);
    c = d.flag.GetValue(); s << c;
    PI(locationsUnlocked); PI(int168);
    unsigned n = (unsigned)d.unopenedSilverBoxes.size();
    s << n;
    for (size_t k = 0; k < d.unopenedSilverBoxes.size(); k++) { i = d.unopenedSilverBoxes[k].GetValue(); s << i; }
    PI(goldenBoxes); PI(firstMissionComplete); PI(meenaRamTutoDisplayed); PI(reviveTutoComplete); PI(firstPressPageButton); PI(dailyCurTime);
    PI(dailyDays); PI(getFirstCostume); PI(getSecondCostume); PI(awardCostumeTime1); PI(awardCostumeTime2); PI(magicCostumeTime);
    PI(isMagicCostume); PI(sideLookTutorialDone); PI(lastLuckyDrawDay); PI(firstLoginDay); PI(notFirstDraw); PI(firstBuy);
#undef PI
    n = (unsigned)d.redeemCostumes.size();
    s << n;
    for (std::map<short, int>::const_iterator it = d.redeemCostumes.begin(); it != d.redeemCostumes.end(); ++it) {
        short k = it->first;
        int v = it->second;
        s << k << v;
    }
    sh = d.bossTutorials.GetValue(); s << sh;
}

// LoadDataFromStream @0x51c784: versions 1..5 add fields to the end of the previous one
bool Player::LoadDataFromStream(jet::stream::IStream &s, PlayerSaveData &d) const {
    int version = 0;
    s >> version;
    if (version < 1 || version > 5) return false;
    int sig = 0;
    s >> sig;
    static const int sigs[6] = { 0, k_signatureV1, k_signatureV2, k_signatureV3, k_signatureV4, k_signatureV5 };
    if (sig != sigs[version]) return false;
    int i;
    short sh;
    char c;
#define RI(f) s >> i; d.f.SetValue(i)
    RI(startTokensGiven); RI(bananas); RI(tokens);
    s >> sh; d.shortA.SetValue(sh);
    s >> sh; d.shortB.SetValue(sh);
    RI(intF0); RI(int118);
    if (version >= 3) { RI(int140); }
    s >> c; d.flag.SetValue(c);
    if (version >= 2) { RI(locationsUnlocked); }
    if (version >= 4) {
        RI(int168);
        unsigned n = 0;
        s >> n;
        d.unopenedSilverBoxes.assign(n, PInt());
        for (unsigned k = 0; k < n; k++) { s >> i; d.unopenedSilverBoxes[k].SetValue(i); }
        RI(goldenBoxes); RI(firstMissionComplete); RI(meenaRamTutoDisplayed); RI(reviveTutoComplete); RI(firstPressPageButton); RI(dailyCurTime);
        RI(dailyDays); RI(getFirstCostume); RI(getSecondCostume); RI(awardCostumeTime1); RI(awardCostumeTime2); RI(magicCostumeTime);
        RI(isMagicCostume); RI(sideLookTutorialDone); RI(lastLuckyDrawDay); RI(firstLoginDay); RI(notFirstDraw); RI(firstBuy);
        unsigned m = 0;
        s >> m;
        for (unsigned k = 0; k < m; k++) {
            short key;
            int v;
            s >> key >> v;
            d.redeemCostumes[key] = v;
        }
    }
    if (version >= 5) { s >> sh; d.bossTutorials.SetValue(sh); }
#undef RI
    return true;
}

bool Player::HasEnoughMoney(int amount, int priceType) const {
    if (priceType == 0) return amount <= m_data.bananas.GetValue();
    if (priceType == 1) return amount <= m_data.tokens.GetValue();
    return true;
}

// @0x423840: the price in bananas or tokens
bool Player::ExecuteCost(const Price &price, std::function<void(int, int)> onMoneyBought, std::function<void()> onCancel) {
    return ExecuteCostManual((int)price.GetPriceAsDouble(), price.GetPriceType(), onMoneyBought, onCancel, price.GetItemName(), price.GetAmount(), price.GetCategory());
}

// @0x423940: a price is paid when there is that much of the currency; a player short of it is offered to buy more
// (CheckMoney). Not ported: what the online services are told of the purchase (the CRM points, the costumes and
// upgrades in the inventory of the online profile); the profile is marked to be updated
bool Player::ExecuteCostManual(int cost, int priceType, std::function<void(int, int)> onMoneyBought, std::function<void()> onCancel, const jet::String&, int, const jet::String&) {
    if (cost <= 0) return false;
    if (!CheckMoney(cost, priceType, onMoneyBought, onCancel)) return false;
    if (priceType == Price::Bananas) AddBananas(-cost);
    else if (priceType == Price::Tokens) AddTokens(-cost);
    Game::GetInstance()->UpdateProfile();
    return true;
}

// @0x423b40: enough of the currency, or not: the .so then shows the popup that sells the missing amount for real money
// (PopupCurrency, or STORE_NO_PACKS_AVAILABLE when the store has nothing); popups are not ported, nothing is bought
bool Player::CheckMoney(int cost, int priceType, std::function<void(int, int)>, std::function<void()>) const { return HasEnoughMoney(cost, priceType); }

void Player::AddUnopenedBlindBox(const jet::String&, int type) {
    if (type < 0) m_data.goldenBoxes.SetValue(m_data.goldenBoxes.GetValue() + 1);
    else m_data.unopenedSilverBoxes.push_back(PInt(type));
}

void Player::TrySettingUserStartTokens2() {
    if (m_data.startTokensGiven.GetValue() != 0) return;
    m_data.startTokensGiven.SetValue(1);
    AddTokens(50);
}

// @0x423f58: the first location is always there, the others unlock by the conditions of the data; a location unlocks the ones
// associated with it
void Player::ComputeLocationUnlock() {
    SetLocationUnlocked(0);
    for (int l = 1; l <= 5; l++) ComputeLocationUnlock(l);
    LocationsMgr *lm = LocationsMgr::GetInstance();
    for (int l = 0; l < ELocationDef::COUNT; l++) {
        LocationInfoEntity *info = lm->GetLocationInfo(l);
        if (!info || !IsLocationUnlocked(info->GetLocationId())) continue;
        const std::vector<int> &assoc = lm->GetAssociatedLocationsFor(info->GetLocationId());
        for (size_t i = 0; i < assoc.size(); i++)
            if (!IsLocationUnlocked(assoc[i])) SetLocationUnlocked(assoc[i]);
    }
}

// @0x424070: a location whose condition is met unlocks, and a player who finished the first run is told of it (a hidden
// location is not announced). The .so also flags the page of the sort help (SortPageHelp, not ported)
void Player::ComputeLocationUnlock(int location) {
    if (IsLocationUnlocked(location)) return;
    LocationInfoEntity *info = LocationsMgr::GetInstance()->GetLocationInfo(location);
    if (!info->IsLocationUnlocked()) return;
    SetLocationUnlocked(location);
    if (info->IsHidden()) return;
    if (TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay)) PopupMgr::GetInstance()->PushPopup(new PopupGoToLocation(location, true, std::function<void()>()));
    SortPageHelp::GetInstance()->m_locationUnlocked = true;
}

// @0x423da8
int Player::AddAmountFrom(const Price &price) {
    int amount = 0;
    if (price.GetPriceType() == Price::Tokens) {
        amount = price.GetAmount();
        AddTokens(amount);
    } else if (price.GetPriceType() == Price::Bananas) {
        amount = price.GetAmount();
        AddBananas(amount);
    } else {
        return 0;
    }
    Game::GetInstance()->UpdateProfile();
    return amount;
}
