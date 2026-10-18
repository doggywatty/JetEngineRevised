// source/game/gamemanagers/COPPAMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_COPPAMGR_H
#define SOURCE_GAME_GAMEMANAGERS_COPPAMGR_H

#include "jsingleton.h"
#include "core/pstring.h"
#include "game/utils/GameSaveable.h"

// What the age gate keeps (saved as the record "COPPAMgr" of the settings file, GameSettings::SaveGameSettings @0x50ad80)
struct COPPAMgrSaveData {  // 0x18 bytes
    unsigned flags = 0;                 // 0x0: ECOPPAFlagsDef bits
    unsigned prevFlags = 0;             // 0x4: what the flags were before (PrevValue_SetFlag @0x546b5c)
    unsigned lastEmailQuestionTime = 0; // 0x8: the server time of the last question of the parents' mail (SetLastEmailQuestionTime)
    int birthYear = 0, birthMonth = 0, birthDay = 0;  // 0xC, 0x10, 0x14 (SetBirthday @0x546568, ResetBirthdayQuestion)
};

// The age gate and the country rules (COPPA, OFT, Italy, AU/NZ). Ported so far: the device country, the country table,
// the flags (and their save record) and the questions the options and goals menus ask; not yet: the birthday / gender
// questions and Update's popups.
class COPPAMgr : public Singleton<COPPAMgr>, public GameSaveable<COPPAMgrSaveData> {  // sizeof 0x4C
public:
    // the rules of a country (COPPAMgr::CountryInfo::Make* @0x545e1c)
    struct CountryInfo {  // sizeof 0x14
        jet::String code;   // 0x0
        int minimumAge;     // 0x8
        bool oft;           // 0xC: the OFT rules (the contact section is shown below the minimum age)
        bool oftD, oftE;    // 0xD, 0xE
        bool italy;         // 0xF: the IAP disclaimer
        bool oft10;         // 0x10
        bool auNz;          // 0x11
        bool coppa;         // 0x12
    };
    COPPAMgr();
    virtual ~COPPAMgr() {}
    void TryToEnableCOPPA();
    jet::String GetDeviceCountry();
    const CountryInfo *GetCountryInfo() const;
    bool AreSocialFeaturesEnabled() const;
    bool MustShowContactSection() const;
    bool IsMinimumAgeMet() const { return HasFlag(2); }
    bool IsItaliaCountry() const;
    bool HasFlag(unsigned f) const { return (m_data.flags & f) == f; }  // @0x546b14
    void SetFlag(unsigned f, bool on) { m_data.flags = on ? (m_data.flags | f) : (m_data.flags & ~f); }  // @0x546b2c
    virtual void ResetData(COPPAMgrSaveData&) const;
    virtual void SaveDataInStream(jet::stream::IStreamW&, COPPAMgrSaveData&) const;
    virtual bool LoadDataFromStream(jet::stream::IStream&, COPPAMgrSaveData&) const;
    static const jet::String k_saveRecordName;  // @0xd0e694: "COPPAMgr"
    static const int k_saveGameVersion = 1, k_signatureV1 = 0xaa0002;
    static jet::String FixEURSymbol(const jet::String&);
private:
    void SerializeV1(jet::stream::IStreamW&, COPPAMgrSaveData&) const;  // @0x546148
    bool DeserializeV1(jet::stream::IStream&, COPPAMgrSaveData&) const;  // @0x5461d4
    jet::String m_country;  // 0x44: the ISO code of the device's country
};

#endif
