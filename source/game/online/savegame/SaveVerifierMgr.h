// source/game/online/savegame/SaveVerifierMgr.h
#ifndef SOURCE_GAME_ONLINE_SAVEGAME_SAVEVERIFIERMGR_H
#define SOURCE_GAME_ONLINE_SAVEGAME_SAVEVERIFIERMGR_H

#include "jsingleton.h"
#include "core/pstring.h"
#include "recorddb.h"

// Binds a save game to the device that wrote it: the record "SaveVerifierMgr" of the save holds the device's encryption
// GLUID, and a save whose record is not the device's (a file copied from another device, or one that was edited) is
// refused when the game loads it (Game::LoadFromRecordDB).
class SaveVerifierMgr : public Singleton<SaveVerifierMgr> {  // sizeof 0x4
public:
    SaveVerifierMgr() {}
    virtual ~SaveVerifierMgr() {}
    // @0x4f9d74: writes the device's GLUID in the save
    void WriteValidSave(clara::RecordDB &db);
    // @0x4f9ea0: the save's GLUID is the device's
    bool IsValidSave(clara::RecordDB &db);
    // gaia::GameloftID::RetrieveEncryptionGLUID() @0x8d7a64 as a string (gaia::GLUID::toString @0x8d994c: its three
    // values and its generation algorithm, separated by spaces). The .so keeps the GLUID in the Android key store
    // (made at the first launch from the hardware id, Android_Generate_GLUIDs_At_First_Launch @0x8d66bc); here it is
    // made at the first launch too and kept in the file "gluid_enc" of the user folder (System::GetUserFolder).
    static jet::String RetrieveEncryptionGLUID();

    static const jet::String k_recordName;  // @0xd0de38: "SaveVerifierMgr"
};

#endif
