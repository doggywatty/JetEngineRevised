// source/game/gamemanagers/TauntsMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_TAUNTSMGR_H
#define SOURCE_GAME_GAMEMANAGERS_TAUNTSMGR_H

class TauntsMgr : public Singleton<TauntsMgr>, public GameSaveable<TauntsMgrSaveData> {  // sizeof 0x3C
public:
    TauntsMgr();
    virtual ~TauntsMgr();
    void Reset();
    virtual void ResetData(TauntsMgrSaveData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, TauntsMgrSaveData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, TauntsMgrSaveData&);
    void OnTauntBought(const jet::String&);
    void OnSetDefaultTaunt(const jet::String&);
    const jet::String &GetUserDefaultTauntName();
    const jet::String &GetInitialDefaultTauntName();
private:
    void SerializeV1(jet::stream::IStreamW&, TauntsMgrSaveData&);
    bool DeserializeV1(jet::stream::IStream&, TauntsMgrSaveData&);
public:
    static const jet::String k_saveRecordName;
private:
    static const int k_saveGameVersion;
    static const int k_signatureV1;
    jet::String m_initialDefaultTauntName;  // 0x38
};

#endif
