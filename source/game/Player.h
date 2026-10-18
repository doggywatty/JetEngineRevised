// source/game/Player.h
#ifndef SOURCE_GAME_PLAYER_H
#define SOURCE_GAME_PLAYER_H

#include <functional>
#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/protectedstorage.h"
#include "utils/GameSaveable.h"

class Price;

typedef jet::core::ProtectedStorage<int> PInt;

// What is saved of the player; every value is a ProtectedStorage (0x28 bytes each) at the offset noted
struct PlayerSaveData {
    PInt startTokensGiven;                 // 0x00
    PInt bananas;                          // 0x28
    PInt tokens;                           // 0x50
    jet::core::ProtectedStorage<short> shortA, shortB;  // 0x78, 0xa0
    jet::core::ProtectedStorage<char> flag;  // 0xc8
    PInt intF0, int118, int140, int168;    // 0xf0 0x118 0x140 0x168
    PInt locationsUnlocked;                // 0x190
    std::vector<PInt> unopenedSilverBoxes; // 0x1b8
    PInt goldenBoxes;                      // 0x1c4
    PInt firstMissionComplete;             // 0x1ec
    PInt meenaRamTutoDisplayed;            // 0x214
    PInt reviveTutoComplete;               // 0x23c
    PInt firstPressPageButton;             // 0x264
    PInt dailyCurTime;                     // 0x28c
    PInt dailyDays;                        // 0x2b4
    PInt getFirstCostume;                  // 0x2dc
    PInt getSecondCostume;                 // 0x304
    PInt awardCostumeTime1;                // 0x32c
    PInt awardCostumeTime2;                // 0x354
    PInt magicCostumeTime;                 // 0x37c
    PInt isMagicCostume;                   // 0x3a4
    PInt sideLookTutorialDone;             // 0x3cc
    PInt lastLuckyDrawDay;                 // 0x3f4
    PInt firstLoginDay;                    // 0x41c
    PInt notFirstDraw;                     // 0x444
    PInt firstBuy;                         // 0x46c
    std::map<short, int> redeemCostumes;   // 0x494
    // the boss tutorials done: a bit per boss (the bit is the boss's own, see BossGameplay::IsTutorialCompleted)
    jet::core::ProtectedStorage<short> bossTutorials;  // version 5
};

// The profile: currencies, unlocked locations, tutorials and daily bonuses, saved as record "Player"
class Player : public Singleton<Player>, public GameSaveable<PlayerSaveData> {
public:
    Player();
    virtual ~Player();
    virtual void ResetData(PlayerSaveData&) const;
    virtual void SaveDataInStream(jet::stream::IStreamW&, PlayerSaveData&) const;
    virtual bool LoadDataFromStream(jet::stream::IStream&, PlayerSaveData&) const;

    int GetBananas() const { return m_data.bananas.GetValue(); }
    int GetTokens() const { return m_data.tokens.GetValue(); }
    void AddBananas(int n) { Add(m_data.bananas, n); }
    void AddTokens(int n) { Add(m_data.tokens, n); }
    // the revives owned (the .so's Player+0x170, the storage saved as the last int of the save data): Revive_Number, and the revive bought with it
    int GetReviveQuantity() const { return m_data.int168.GetValue(); }
    void AddReviveQuantity(int n) { Add(m_data.int168, n); }
    // currency 0 bananas, 1 tokens; anything else is always affordable
    bool HasEnoughMoney(int amount, int priceType) const;
    // pays a price in bananas or tokens (true when it was paid); onMoneyBought is what the popup that sells the missing
    // currency calls after a purchase (that popup is not ported, so it is never called)
    // @0x423da8: the amount of a prize of bananas or tokens is given; the amount is returned (0 for another kind)
    int AddAmountFrom(const Price&);
    bool ExecuteCost(const Price&, std::function<void(int, int)> onMoneyBought = std::function<void(int, int)>(), std::function<void()> onCancel = std::function<void()>());
    bool ExecuteCostManual(int cost, int priceType, std::function<void(int, int)> onMoneyBought, std::function<void()> onCancel, const jet::String &itemName, int amount, const jet::String &category);

    // the location the player runs in (ELocation)
    int GetCurrentLocation() const { return m_data.flag.GetValue(); }
    void SetCurrentLocation(int l) { m_data.flag.SetValue((char)l); }
    bool IsLocationUnlocked(int loc) const { return (m_data.locationsUnlocked.GetValue() & (1 << loc)) != 0; }
    // @0x423f58: the locations the data's conditions unlock (a location that was just unlocked is announced by a popup)
    void ComputeLocationUnlock();
    void ComputeLocationUnlock(int location);  // @0x424070
    void SetLocationUnlocked(int loc) { m_data.locationsUnlocked.SetValue(m_data.locationsUnlocked.GetValue() | (1 << loc)); }

    void AddUnopenedBlindBox(const jet::String&, int type);
    int GetLastUnopenedSilverBlindBox() const { return m_data.unopenedSilverBoxes.back().GetValue(); }
    void SetLastSilverBlindBoxOpened() { m_data.unopenedSilverBoxes.pop_back(); }
    unsigned CountUnopenedSilverBlindBoxes() const { return (unsigned)m_data.unopenedSilverBoxes.size(); }
    void SetLastGoldenBlindBoxOpened() { if (m_data.goldenBoxes.GetValue() > 0) m_data.goldenBoxes.SetValue(m_data.goldenBoxes.GetValue() - 1); }
    int CountUnopenedGoldenBlindBoxes() const { return m_data.goldenBoxes.GetValue(); }

    bool GetFirstPressPageButton() const { return m_data.firstPressPageButton.GetValue() == 1; }
    void SetFirstPressPageButton(int v) { m_data.firstPressPageButton.SetValue(v != 0); }
    int GetDailyCurTime() const { return m_data.dailyCurTime.GetValue(); }
    void SetDailyCurTime(int v) { m_data.dailyCurTime.SetValue(v); }
    int GetDailyDays() const { return m_data.dailyDays.GetValue(); }
    void SetDailyDays(int v) { m_data.dailyDays.SetValue(v); }
    int IsGetFirstCostume() const { return m_data.getFirstCostume.GetValue(); }
    void SetGetFirstCostume(int v) { m_data.getFirstCostume.SetValue(v); }
    int IsGetSecondCostume() const { return m_data.getSecondCostume.GetValue(); }
    void SetGetSecondCostume(int v) { m_data.getSecondCostume.SetValue(v); }
    void SetAwardCostumeTime_1(int v) { m_data.awardCostumeTime1.SetValue(v); }
    int GetAwardCostumeTime_1() const { return m_data.awardCostumeTime1.GetValue(); }
    void SetAwardCostumeTime_2(int v) { m_data.awardCostumeTime2.SetValue(v); }
    int GetAwardCostumeTime_2() const { return m_data.awardCostumeTime2.GetValue(); }
    void SetMagicCostumeTime(int v) { m_data.magicCostumeTime.SetValue(v); }
    int GetMagicCostumeTime() const { return m_data.magicCostumeTime.GetValue(); }
    int IsMigicCostume() const { return m_data.isMagicCostume.GetValue(); }
    void SetIsMigicCostume(int v) { m_data.isMagicCostume.SetValue(v); }
    bool IsSideLookTutorialDone() const { return m_data.sideLookTutorialDone.GetValue() == 1; }
    void SetSideLookTutorialDone(bool v) { m_data.sideLookTutorialDone.SetValue(v); }
    int GetLastLucklyDrawDay() const { return m_data.lastLuckyDrawDay.GetValue(); }
    void SetLastLucklyDrawDay(int v) { m_data.lastLuckyDrawDay.SetValue(v); }
    int GetFirstLoginDay() const { return m_data.firstLoginDay.GetValue(); }
    void SetFirstLoginDay(int v) { m_data.firstLoginDay.SetValue(v); }
    bool GetNotFirstDraw() const { return m_data.notFirstDraw.GetValue() == 1; }
    void SetNotFirstDraw(bool v) { m_data.notFirstDraw.SetValue(v); }
    bool GetFirstBuy() const { return m_data.firstBuy.GetValue() != 1; }
    void SetFirstBuy(bool v) { m_data.firstBuy.SetValue(!v); }
    bool GetFirstMissionComplete() const { return m_data.firstMissionComplete.GetValue() == 1; }
    void SetFirstMissionComplete(bool v) { m_data.firstMissionComplete.SetValue(v); }
    bool GetMeenaRamTutoDisplayed() const { return m_data.meenaRamTutoDisplayed.GetValue() == 1; }
    void SetMeenaRamTutoDisplayed(bool v) { m_data.meenaRamTutoDisplayed.SetValue(v); }
    bool GetReviveTutoComplete() const { return m_data.reviveTutoComplete.GetValue() == 1; }
    void SetReviveTutoComplete(bool v) { m_data.reviveTutoComplete.SetValue(v); }
    std::map<short, int> GetRedeemCostume() const { return m_data.redeemCostumes; }
    void AddRedeemCostume(short id, int v) { m_data.redeemCostumes[id] = v; }
    void RemoveRedeemCostume(short id) { m_data.redeemCostumes.erase(id); }
    // the boss tutorials: the bit of a boss is set when its tutorial is completed (BossGameplay::CompleteTutorial @0x5a60d0)
    short GetBossTutorials() const { return m_data.bossTutorials.GetValue(); }
    void SetBossTutorials(short v) { m_data.bossTutorials.SetValue(v); }
    int FindRedeemCostume(short id) const { std::map<short, int>::const_iterator i = m_data.redeemCostumes.find(id); return i == m_data.redeemCostumes.end() ? 0 : i->second; }

    // TrySettingUserStartTokens2 @0x51dd20: a new profile starts with 50 tokens
    void TrySettingUserStartTokens2();

    static const jet::String k_saveRecordName;  // @0xd0ad78: "Player"
    static const int k_saveGameVersion = 5;
    static const int k_signatureV1 = 0xaa000f, k_signatureV2 = 0xaa0017, k_signatureV3 = 0xaa0001, k_signatureV4 = 0xaa0002, k_signatureV5 = 0xaa0003;
private:
    bool CheckMoney(int cost, int priceType, std::function<void(int, int)> onMoneyBought, std::function<void()> onCancel) const;
    static void Add(PInt &p, int n) {
        p.SetValue(p.GetValue() + n);
        if (p.GetValue() < 0) p.SetValue(0);
    }
};

#endif
