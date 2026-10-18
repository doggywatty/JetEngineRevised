// source/game/utils/GameSaveable.h
#ifndef SOURCE_GAME_UTILS_GAMESAVEABLE_H
#define SOURCE_GAME_UTILS_GAMESAVEABLE_H

#include "recorddb.h"
#include "stream/memorystream.h"
#include "stream/memlinkstream.h"

// Something saved as a binary record of a RecordDB: a live copy of its data and a backup one (@0x53e200)
template <class T>
class GameSaveable {
public:
    virtual ~GameSaveable() {}
    virtual bool LoadDataFromStream(jet::stream::IStream&, T&) const = 0;
    virtual void SaveDataInStream(jet::stream::IStreamW&, T&) const = 0;
    virtual void ResetData(T&) const = 0;

    void ResetSaveable(bool backup) { ResetData(backup ? m_backup : m_data); }
    void SaveToRecordDB(clara::RecordDB &db, const jet::String &name, bool backup) {
        jet::stream::MemoryStream ms;
        ms.Start();
        SaveDataInStream(ms, backup ? m_backup : m_data);
        ms.SeekBeg(0);
        clara::Record r(clara::Record::TypeDef::BINARY);
        if (ms.Size()) r.Set(ms.GetInternalArray().data(), ms.Size());
        db.Set(name, r);
        ms.Finish();
    }
    bool LoadFromRecordDB(clara::RecordDB &db, const jet::String &name, bool backup) {
        const clara::Record &r = db.Get(name);
        const std::vector<unsigned char> &bin = r.GetAsBinary();
        if (r.GetType() != clara::Record::TypeDef::BINARY || bin.empty()) return false;
        jet::stream::MemLinkStream s(bin);
        s.Start();
        bool ok = s.Size() != 0;
        if (ok) {
            ResetSaveable(backup);
            ok = LoadDataFromStream(s, backup ? m_backup : m_data);
        }
        s.Finish();
        return ok;
    }
    T &GetData() { return m_data; }
    T &GetBackupData() { return m_backup; }
protected:
    T m_data;    // 0x4
    T m_backup;  // 0x4b0
};

#endif
