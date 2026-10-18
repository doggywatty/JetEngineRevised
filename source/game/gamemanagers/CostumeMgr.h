// source/game/gamemanagers/CostumeMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_COSTUMEMGR_H
#define SOURCE_GAME_GAMEMANAGERS_COSTUMEMGR_H

#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "game/utils/GameSaveable.h"

class MinionCostume;
class Price;

// What the player owns of the costumes (saved): the equipped one's name, the ones bought, the puzzle pieces found
// of the "puzzle" costumes, and the points factor the bought ones add, which is kept so it need not be summed.
struct CostumeMgrSaveData {
    jet::String equipped;                                     // 0x0
    std::vector<jet::String> bought;                          // 0x4
    int extraDespicablePointsFactor = 0;                      // 0x10
    std::map<jet::String, std::vector<unsigned> > puzzlePieces;  // 0x14
};

// The costumes of the data (the entities of the "MinionCostumes" folder) and which one the minion wears. 0x78 bytes in
// the .so. Not ported: the "costumes are locked" popup of SetEquippedCostume, the tracking event of a completed puzzle.
class CostumeMgr : public Singleton<CostumeMgr>, public GameSaveable<CostumeMgrSaveData> {
public:
    CostumeMgr();
    virtual ~CostumeMgr();
    void Reset();
    void Update() {}
    unsigned GetCostumeRefCount() const { return (unsigned)m_costumesRefs.size(); }
    MinionCostume *GetCostumeRef(unsigned i) const { return m_costumesRefs[i]; }
    MinionCostume *GetEquippedCostumeRef() const { return m_equippedCostumeRef; }
    // the byte at +0x74: Reset made the costumes of the data (the shop has nothing to show before)
    bool WasReset() const { return m_firstReset; }
    // the end of a run sets the same byte (GameLevel::OnMinionNoRevive @0x53ef1c stores 1 at +0x74)
    void MarkWasReset() { m_firstReset = true; }
    void ClearWasReset() { m_firstReset = false; }  // @0x4fce78: the gameplay's start clears it (the wear guard of SetEquippedCostume)
    MinionCostume *GetPlaceHolderCostumeRef() const { return m_placeholderCostumeRef; }
    void SetEquippedCostume(MinionCostume*, bool force);
    void MarkAsBought(const MinionCostume&);
    bool WasBought(const MinionCostume&) const;
    bool IsPriceBought(const Price*) const;
    const jet::String &GetEquippedCostumeName() const { return m_data.equipped; }
    unsigned GetCountBoughtCostumes() const { return (unsigned)m_data.bought.size(); }
    unsigned GetCountBoughtCostumesNotAvailableInData() const;
    void SetEquippedCostumeNameFrom(MinionCostume*);
    bool IsCurrentCostumeADressup() const;
    MinionCostume *FindCostumeRefByName(const jet::String&) const;
    MinionCostume *GetCostumeById(unsigned index);
    void RemoveCostumeById(unsigned index);
    int GetGlobalExtraDespicablePointsFactor(bool backup) const;
    void SetCostumesVisible(bool);
    // the puzzles
    void Puzzle_AddNewPieceFor(const MinionCostume&, unsigned piece);
    int Puzzle_GetRandomNotOwnedPiece(const MinionCostume&) const;
    int Puzzle_GetPiecesOwnedCountFor(const MinionCostume&) const;
    const std::vector<unsigned> &Puzzle_GetPiecesOwnedFor(const MinionCostume&) const;
    bool Puzzle_AnyPuzzlePiecesLeft() const;
    MinionCostume *Puzzle_AddPuzzlePieceToRandomCostume();
    bool Puzzle_WasPuzzleCompletedFor(const MinionCostume&) const;
    void Puzzle_IfCompletedMarkAsBought(const MinionCostume&);
    virtual void ResetData(CostumeMgrSaveData&) const;
    virtual void SaveDataInStream(jet::stream::IStreamW&, CostumeMgrSaveData&) const;
    virtual bool LoadDataFromStream(jet::stream::IStream&, CostumeMgrSaveData&) const;
    static const jet::String k_saveRecordName, k_folderName, k_defaultCostumeName;
private:
    void ClearAll();
    void SerializeV1(jet::stream::IStreamW&, CostumeMgrSaveData&) const;
    bool DeserializeV1(jet::stream::IStream&, CostumeMgrSaveData&) const;
    void SerializeV2(jet::stream::IStreamW&, CostumeMgrSaveData&) const;
    bool DeserializeV2(jet::stream::IStream&, CostumeMgrSaveData&) const;
    void SerializeV3(jet::stream::IStreamW&, CostumeMgrSaveData&) const;
    bool DeserializeV3(jet::stream::IStream&, CostumeMgrSaveData&) const;
    std::vector<MinionCostume*> m_costumesRefs;  // 0x60
    MinionCostume *m_equippedCostumeRef;         // 0x6C
    MinionCostume *m_placeholderCostumeRef;      // 0x70
    bool m_firstReset;                           // 0x74
    static const int k_saveGameVersion = 3;
    static const int k_signatureV1 = 0xaa0002, k_signatureV2 = 0xaa0003, k_signatureV3 = 0xaa0004;
};

#endif
