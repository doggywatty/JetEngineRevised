// source/game/gamemanagers/RedeemCodeMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_REDEEMCODEMGR_H
#define SOURCE_GAME_GAMEMANAGERS_REDEEMCODEMGR_H

class RedeemCodeMgr : public Singleton<RedeemCodeMgr>, public GameSaveable<RedeemCodeSaveData> {  // sizeof 0x40
public:
    RedeemCodeMgr();
    virtual ~RedeemCodeMgr();
    void Reset();
    RedeemCodeMgr::SResult ValidateEntry(const ustl::vector<int>&, bool);
    virtual void ResetData(RedeemCodeSaveData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, RedeemCodeSaveData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, RedeemCodeSaveData&);
private:
    bool WasRedeemCodeNameUsed(const jet::String&);
    void UseRedeemCodeName(const jet::String&);
    void SerializeV1(jet::stream::IStreamW&, RedeemCodeSaveData&);
    bool DeserializeV1(jet::stream::IStream&, RedeemCodeSaveData&);
public:
    static const jet::String k_saveRecordName;
private:
    static const int k_saveGameVersion;
    ustl::vector<RedeemCode *> m_redeemCodesArray;  // 0x30
    static const int k_signatureV1;
};

#endif
