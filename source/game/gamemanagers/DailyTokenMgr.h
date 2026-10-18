// source/game/gamemanagers/DailyTokenMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_DAILYTOKENMGR_H
#define SOURCE_GAME_GAMEMANAGERS_DAILYTOKENMGR_H

class DailyTokenMgr : public Singleton<DailyTokenMgr>, public GameSaveable<TokenDealerData> {  // sizeof 0x48
public:
    bool ShouldCreateToken(GameEntity*);
    void CreateToken(GameEntity*);
    void RefreshTokenDealers();
    void OnTokenCollected(Token*);
    virtual void ResetData(TokenDealerData&);
    virtual void SaveDataInStream(jet::stream::IStreamW&, TokenDealerData&);
    virtual bool LoadDataFromStream(jet::stream::IStream&, TokenDealerData&);
private:
    TokenDealerData::TokenDealer *FindTokenDealerByDistance(float);
    bool UpdateServerTime(unsigned&);
    void SerializeV1(jet::stream::IStreamW&, TokenDealerData&);
    bool DeserializeV1(jet::stream::IStream&, TokenDealerData&);
public:
    static const jet::String k_saveRecordName;
    static const int k_saveGameVersion;
    static const TokenDealerDataConfig k_dealerConfig[0];
    static const int k_dealerConfigCount;
private:
    static const int k_signatureV1;
};

#endif
