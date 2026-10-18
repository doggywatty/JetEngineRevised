// source/game/gamemanagers/BonusUpgradeMgr.cpp
#include "BonusUpgradeMgr.h"
#include "game/Game.h"
#include "game/entities/Bonus.h"
#include "game/entities/BonusSet.h"
#include "game/Statistics.h"
#include "game/online/store/Price.h"

const jet::String BonusUpgradeMgr::k_saveRecordName("BonusUpgradeMgr");  // @0xc3fdda

// @0x543f34
BonusUpgradeMgr::BonusUpgradeMgr() : m_bonusSet(0) {}

// @0x544028: every bonus of the set takes the level saved under its name, or its first one when none is saved
void BonusUpgradeMgr::Reset() {
    m_bonusSet = BonusSet::GetInstance();
    if (!m_bonusSet) return;
    for (Bonus *b : m_bonusSet->GetBonusesArray()) {
        bool found = false;
        for (const BonusUpgradeMgrSaveData::BonusInfo &info : m_data.bonuses)
            if (b->GetName().Equals(info.name)) {
                b->m_level = info.level;
                found = true;
                break;
            }
        if (!found) b->ResetLevelToDefault();
    }
}

// @0x54411c
void BonusUpgradeMgr::ResetData(BonusUpgradeMgrSaveData &d) const { d.bonuses.clear(); }

// @0x544144
void BonusUpgradeMgr::SaveDataInStream(jet::stream::IStreamW &s, BonusUpgradeMgrSaveData &d) const {
    int v = k_saveGameVersion;
    s << v;
    SerializeV1(s, d);
}

// @0x544190
bool BonusUpgradeMgr::LoadDataFromStream(jet::stream::IStream &s, BonusUpgradeMgrSaveData &d) const {
    int v = 0;
    s >> v;
    return v == k_saveGameVersion && DeserializeV1(s, d);
}

// @0x5441dc
void BonusUpgradeMgr::SerializeV1(jet::stream::IStreamW &s, BonusUpgradeMgrSaveData &d) const {
    int sig = k_signatureV1;
    s << sig;
    unsigned n = (unsigned)d.bonuses.size();
    s << n;
    for (const BonusUpgradeMgrSaveData::BonusInfo &info : d.bonuses) {
        jet::WriteString(s, info.name);
        s << info.level;
    }
}

// @0x544270
bool BonusUpgradeMgr::DeserializeV1(jet::stream::IStream &s, BonusUpgradeMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    int n = 0;
    s >> n;
    d.bonuses.resize((size_t)n);
    for (int i = 0; i < n; i++) {
        s >> d.bonuses[(size_t)i].name;
        s >> d.bonuses[(size_t)i].level;
    }
    return true;
}

// @0x544338
unsigned BonusUpgradeMgr::GetBonusCount() const { return m_bonusSet->GetBonusCount(); }

// @0x54434c: the level of the bonus is kept under its name
void BonusUpgradeMgr::OnBonusLevelChanged(Bonus *b) {
    for (BonusUpgradeMgrSaveData::BonusInfo &info : m_data.bonuses)
        if (info.name.Equals(b->GetName())) {
            info.level = b->m_level;
            return;
        }
    BonusUpgradeMgrSaveData::BonusInfo info;
    info.name = b->GetName();
    info.level = b->m_level;
    m_data.bonuses.push_back(info);
    Game::GetInstance()->Save(false, false);
}

// @0x5444f0
bool BonusUpgradeMgr::HandleDeliverIAPItem(const Price *p, bool &alreadyBought) {
    alreadyBought = false;
    if (p->GetPriceType() != Price::IAP) return false;
    for (Bonus *b : BonusSet::GetInstance()->GetBonusesArray()) {
        if (!b->HasPriceInPriceArray(p)) continue;
        if (p == b->GetPrice(b->GetCurrentLevel() + 1)) {
            b->OnCurrentLevelPurchased(0, 0);
            Statistics::GetInstance()->AddShopSpentPrice(*p, true);
            return true;
        }
        for (int level = 0; level <= b->GetCurrentLevel(); level++)
            if (p == b->GetPrice(level)) alreadyBought = true;
        return true;
    }
    return false;
}
