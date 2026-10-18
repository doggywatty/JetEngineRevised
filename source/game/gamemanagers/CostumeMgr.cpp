// source/game/gamemanagers/CostumeMgr.cpp
#include "CostumeMgr.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/Player.h"
#include "game/Statistics.h"
#include "game/entities/Deco3d.h"
#include "game/entities/Minion.h"
#include "game/entities/MinionCostume.h"
#include "game/gamestates/GameState.h"
#include "game/online/store/Price.h"
#include "core/util.h"
#include "project.h"
#include "folder.h"
#include "stream/istream.h"
#include <algorithm>

const jet::String CostumeMgr::k_saveRecordName("CostumeMgr");
const jet::String CostumeMgr::k_folderName("MinionCostumes");
const jet::String CostumeMgr::k_defaultCostumeName("00_MinionCostume_None");

// @0x546f5c: the constructor stores 1 at +0x74 (m_firstReset), so the wear guard of SetEquippedCostume is open from the start
CostumeMgr::CostumeMgr() : m_equippedCostumeRef(0), m_placeholderCostumeRef(0), m_firstReset(true) { m_data.equipped = k_defaultCostumeName; }
CostumeMgr::~CostumeMgr() {}

// @0x5470f0
void CostumeMgr::ClearAll() {
    m_equippedCostumeRef = 0;
    m_placeholderCostumeRef = 0;
    m_costumesRefs.clear();
}

// @0x547114: the costumes of the data; the one to wear is the saved one (else the default, else the first), the
// placeholder the last that is not a shop element
void CostumeMgr::Reset() {
    m_firstReset = true;
    ClearAll();
    clara::Folder *folder = clara::Project::GetInstance()->FindFolderByName(k_folderName);
    if (!folder) return;
    MinionCostume *wear = 0, *placeholder = 0;
    bool found = false;
    for (unsigned i = 0; i < folder->GetEntityCount(); i++) {
        MinionCostume *c = static_cast<MinionCostume*>(folder->GetEntity(i));
        m_costumesRefs.push_back(c);
        Puzzle_IfCompletedMarkAsBought(*c);
        if (c->GetClaraEntityName().Equals(GetEquippedCostumeName())) {
            wear = c;
            found = true;
        } else if (!wear && c->GetClaraEntityName().Equals(k_defaultCostumeName)) wear = c;
        if (!c->IsShopElement()) placeholder = c;
    }
    if (!wear) wear = m_costumesRefs[0];
    if (!placeholder) placeholder = wear;
    m_placeholderCostumeRef = placeholder;
    for (MinionCostume *c : m_costumesRefs)
        if (c->IsShopElement()) c->SetPlaceHolderDecoIfNeeded();
    SetEquippedCostume(found ? wear : m_placeholderCostumeRef, false);
    SetCostumesVisible(false);
    m_data.extraDespicablePointsFactor = GetGlobalExtraDespicablePointsFactor(false);
}

// @0x547360: the costumes of the library are not drawn or updated, only the copies the menus make
void CostumeMgr::SetCostumesVisible(bool v) {
    for (MinionCostume *c : m_costumesRefs) {
        c->RegisterForUpdate(v);
        c->RegisterForRender(v);
        if (Deco3d *d = c->GetDeco()) {
            d->RegisterForUpdate(v);
            d->RegisterForRender(v);
        }
    }
    GameLevel::GetInstance()->RemoveEntities();
}

// @0x5473dc: the minion wears the costume (only a shop element is remembered as the equipped one); in the main menu
// it is not drawn by the level, the menu draws it. The .so refuses wearing while the flag is 0 with a popup
// (SHOP_COSTUMES_TAB_FORBIDDEN, PopupMgr is not ported). The flag is 1 from the constructor on, so the refusal is never taken.
void CostumeMgr::SetEquippedCostume(MinionCostume *c, bool force) {
    if (m_equippedCostumeRef == c && !force) return;
    if (!m_firstReset && !force) return;
    m_equippedCostumeRef = FindCostumeRefByName(c->GetClaraEntityName());
    const bool shop = m_equippedCostumeRef->IsShopElement();
    if (shop) SetEquippedCostumeNameFrom(m_equippedCostumeRef);
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    minion->SetCostume(m_equippedCostumeRef->GetCostumeDecoReference(), m_equippedCostumeRef->CanLoadModel() && shop ? m_equippedCostumeRef : 0);
    GameState *st = GameState::GetCrtState();
    if (st && st->GetStateName().Equals("GS_MainMenu") && minion->GetDeco()) {
        minion->GetDeco()->RegisterForRender(false);
        minion->GetDeco()->RegisterForUpdate(false);
    }
}

// @0x54757c
bool CostumeMgr::IsCurrentCostumeADressup() const { return m_equippedCostumeRef && !m_equippedCostumeRef->GetClaraEntityName().Equals(k_defaultCostumeName); }

// @0x5475a8: bought once; the first costume of the day / the second / the magic one are taken off their offers
void CostumeMgr::MarkAsBought(const MinionCostume &c) {
    if (WasBought(c)) return;
    m_data.bought.push_back(c.GetClaraEntityName());
    m_data.puzzlePieces.erase(c.GetClaraEntityName());
    // not ported: the buy activity published to the online player data
    Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_costumesBuyCount, 1);
    Player *pl = Player::GetInstance();
    if (c.GetCostumeId() == 0x12) pl->SetGetFirstCostume(-1);
    if (c.GetCostumeId() == 0x10) pl->SetGetSecondCostume(-1);
    if (c.GetCostumeId() == 0x19) pl->SetIsMigicCostume(-1);
    // the .so saves before the factor is summed again (Game::Save @0x52ddf8, then the store at +0x18)
    Game::GetInstance()->Save(false, false);
    m_data.extraDespicablePointsFactor = GetGlobalExtraDespicablePointsFactor(false);
}

// @0x547730: the default costume is always owned
bool CostumeMgr::WasBought(const MinionCostume &c) const {
    for (const jet::String &n : m_data.bought)
        if (n.Equals(c.GetClaraEntityName())) return true;
    return c.GetClaraEntityName().Equals(k_defaultCostumeName);
}

// @0x548060
bool CostumeMgr::IsPriceBought(const Price *p) const {
    for (MinionCostume *c : m_costumesRefs)
        if (c->GetPrice() == p) return WasBought(*c);
    return false;
}

// @0x54779c: the last of that name
MinionCostume *CostumeMgr::FindCostumeRefByName(const jet::String &n) const {
    MinionCostume *r = 0;
    for (MinionCostume *c : m_costumesRefs)
        if (c->GetClaraEntityName().Equals(n)) r = c;
    return r;
}

// @0x547ff4: no costume is the default; else its library name; else (a library entity with a name) its name. Otherwise unchanged.
void CostumeMgr::SetEquippedCostumeNameFrom(MinionCostume *c) {
    if (!c) {
        m_data.equipped = k_defaultCostumeName;
        return;
    }
    if (!c->GetClaraEntityName().empty()) {
        m_data.equipped = c->GetClaraEntityName();
        return;
    }
    if (c->IsLibraryEntity() && !c->GetName().empty()) m_data.equipped = c->GetName();
}

// @0x5480b0
unsigned CostumeMgr::GetCountBoughtCostumesNotAvailableInData() const {
    unsigned n = 0;
    for (const jet::String &name : m_data.bought)
        if (!FindCostumeRefByName(name)) n++;
    return n;
}

// @0x548650: what the bought costumes add (the saved sum when there are no costumes loaded)
int CostumeMgr::GetGlobalExtraDespicablePointsFactor(bool backup) const {
    if (m_costumesRefs.empty()) return m_data.extraDespicablePointsFactor;
    int sum = 0;
    for (const jet::String &name : (backup ? m_backup : m_data).bought)
        if (MinionCostume *c = FindCostumeRefByName(name)) sum += c->GetExtraDespicablePointsFactorForGlobal();
    return sum;
}

// @0x5486b8: a cheat: the costume is owned and worn
MinionCostume *CostumeMgr::GetCostumeById(unsigned i) {
    MinionCostume *c = m_costumesRefs[i];
    m_data.bought.push_back(c->GetClaraEntityName());
    if (c->CanLoadModel()) SetEquippedCostume(c, false);
    return c;
}

// @0x5486f8: a cheat: the costume is not owned, the minion is back to the regular one (costume 15)
void CostumeMgr::RemoveCostumeById(unsigned i) {
    const jet::String &n = m_costumesRefs[i]->GetClaraEntityName();
    m_data.bought.erase(std::remove_if(m_data.bought.begin(), m_data.bought.end(), [&](const jet::String &b) { return b.Equals(n); }), m_data.bought.end());
    SetEquippedCostume(m_costumesRefs[15], false);
}

// @0x5480f0: a piece is kept once, and only a piece the puzzle has
void CostumeMgr::Puzzle_AddNewPieceFor(const MinionCostume &c, unsigned piece) {
    std::vector<unsigned> &v = m_data.puzzlePieces[c.GetClaraEntityName()];
    if (std::find(v.begin(), v.end(), piece) != v.end()) return;
    if (piece < (unsigned)c.GetPuzzlePiecesCount()) v.push_back(piece);
}

// @0x548164: one of the pieces not owned yet, -1 when there are none
int CostumeMgr::Puzzle_GetRandomNotOwnedPiece(const MinionCostume &c) const {
    std::vector<unsigned> owned;
    auto it = m_data.puzzlePieces.find(c.GetClaraEntityName());
    if (it != m_data.puzzlePieces.end()) owned = it->second;
    std::vector<unsigned> left;
    for (unsigned p = 0; p < (unsigned)c.GetPuzzlePiecesCount(); p++)
        if (std::find(owned.begin(), owned.end(), p) == owned.end()) left.push_back(p);
    if (left.empty()) return -1;
    return (int)left[(unsigned)jet::core::Rand2(0, (int)left.size())];
}

// @0x548364
int CostumeMgr::Puzzle_GetPiecesOwnedCountFor(const MinionCostume &c) const {
    if (WasBought(c)) return c.GetPuzzlePiecesCount();
    auto it = m_data.puzzlePieces.find(c.GetClaraEntityName());
    return it == m_data.puzzlePieces.end() ? 0 : (int)it->second.size();
}

// @0x5483b4
const std::vector<unsigned> &CostumeMgr::Puzzle_GetPiecesOwnedFor(const MinionCostume &c) const {
    static const std::vector<unsigned> none;
    auto it = m_data.puzzlePieces.find(c.GetClaraEntityName());
    return it == m_data.puzzlePieces.end() ? none : it->second;
}

// @0x5483f8: an offered puzzle costume that is not bought and has pieces missing
bool CostumeMgr::Puzzle_AnyPuzzlePiecesLeft() const {
    for (MinionCostume *c : m_costumesRefs)
        if (c->GetBuyType().Equals(MinionCostume::k_buyType_puzzle) && !WasBought(*c) && c->GetPrice()->IsActive() && Puzzle_GetPiecesOwnedCountFor(*c) < c->GetPuzzlePiecesCount()) return true;
    return false;
}

// @0x548498: gives a not owned piece of the first offered puzzle costume that has some; null when there is none
MinionCostume *CostumeMgr::Puzzle_AddPuzzlePieceToRandomCostume() {
    for (MinionCostume *c : m_costumesRefs) {
        if (!c->GetBuyType().Equals(MinionCostume::k_buyType_puzzle) || WasBought(*c) || !c->GetPrice()->IsActive()) continue;
        const int piece = Puzzle_GetRandomNotOwnedPiece(*c);
        if (piece == -1) continue;
        Puzzle_AddNewPieceFor(*c, (unsigned)piece);
        return c;
    }
    return 0;
}

// @0x548564
bool CostumeMgr::Puzzle_WasPuzzleCompletedFor(const MinionCostume &c) const {
    return WasBought(c) || Puzzle_GetPiecesOwnedCountFor(c) >= c.GetPuzzlePiecesCount();
}

// @0x5485ac: a puzzle costume with all its pieces is bought (its tracking event is not sent)
void CostumeMgr::Puzzle_IfCompletedMarkAsBought(const MinionCostume &c) {
    if (WasBought(c) || !c.GetBuyType().Equals(MinionCostume::k_buyType_puzzle)) return;
    if (Puzzle_WasPuzzleCompletedFor(c)) MarkAsBought(c);
}

// @0x5477dc
void CostumeMgr::ResetData(CostumeMgrSaveData &d) const {
    d.equipped = k_defaultCostumeName;
    d.bought.clear();
    d.puzzlePieces.clear();
}

// @0x54783c
void CostumeMgr::SaveDataInStream(jet::stream::IStreamW &s, CostumeMgrSaveData &d) const {
    int v = k_saveGameVersion;
    s << v;
    SerializeV3(s, d);
}

// @0x547888
bool CostumeMgr::LoadDataFromStream(jet::stream::IStream &s, CostumeMgrSaveData &d) const {
    int v = 0;
    s >> v;
    if (v == 1) return DeserializeV1(s, d);
    if (v == 2) return DeserializeV2(s, d);
    if (v == 3) return DeserializeV3(s, d);
    return false;
}

static void WriteBought(jet::stream::IStreamW &s, const CostumeMgrSaveData &d) {
    unsigned n = (unsigned)d.bought.size();
    s << n;
    for (const jet::String &b : d.bought) jet::WriteString(s, b);
}

static void WritePuzzles(jet::stream::IStreamW &s, const CostumeMgrSaveData &d) {
    unsigned n = (unsigned)d.puzzlePieces.size();
    s << n;
    for (auto &kv : d.puzzlePieces) {
        jet::WriteString(s, kv.first);
        unsigned m = (unsigned)kv.second.size();
        s << m;
        for (unsigned p : kv.second) s << p;
    }
}

static void ReadBought(jet::stream::IStream &s, CostumeMgrSaveData &d) {
    int n = 0;
    s >> n;
    d.bought.clear();
    for (int i = 0; i < n; i++) {
        jet::String b;
        s >> b;
        d.bought.push_back(b);
    }
}

static void ReadPuzzles(jet::stream::IStream &s, CostumeMgrSaveData &d) {
    unsigned n = 0;
    s >> n;
    for (unsigned i = 0; i < n; i++) {
        jet::String key;
        s >> key;
        unsigned m = 0;
        s >> m;
        std::vector<unsigned> v;
        for (unsigned k = 0; k < m; k++) {
            int p = 0;
            s >> p;
            v.push_back((unsigned)p);
        }
        d.puzzlePieces[key] = v;
    }
}

// @0x54790c
void CostumeMgr::SerializeV1(jet::stream::IStreamW &s, CostumeMgrSaveData &d) const {
    int sig = k_signatureV1;
    s << sig;
    jet::WriteString(s, d.equipped);
    WriteBought(s, d);
}

// @0x54798c
bool CostumeMgr::DeserializeV1(jet::stream::IStream &s, CostumeMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    s >> d.equipped;
    if (d.equipped.empty()) d.equipped = k_defaultCostumeName;
    ReadBought(s, d);
    return true;
}

// @0x547a6c
void CostumeMgr::SerializeV2(jet::stream::IStreamW &s, CostumeMgrSaveData &d) const {
    int sig = k_signatureV2;
    s << sig;
    jet::WriteString(s, d.equipped);
    WriteBought(s, d);
    WritePuzzles(s, d);
}

// @0x547b7c
bool CostumeMgr::DeserializeV2(jet::stream::IStream &s, CostumeMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV2) return false;
    s >> d.equipped;
    if (d.equipped.empty()) d.equipped = k_defaultCostumeName;
    ReadBought(s, d);
    ReadPuzzles(s, d);
    return true;
}

// @0x547d20: the factor the bought costumes add is saved with them
void CostumeMgr::SerializeV3(jet::stream::IStreamW &s, CostumeMgrSaveData &d) const {
    int sig = k_signatureV3;
    s << sig;
    jet::WriteString(s, d.equipped);
    s << d.extraDespicablePointsFactor;
    WriteBought(s, d);
    WritePuzzles(s, d);
}

// @0x547e3c
bool CostumeMgr::DeserializeV3(jet::stream::IStream &s, CostumeMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV3) return false;
    s >> d.equipped;
    if (d.equipped.empty()) d.equipped = k_defaultCostumeName;
    s >> d.extraDespicablePointsFactor;
    ReadBought(s, d);
    ReadPuzzles(s, d);
    return true;
}
