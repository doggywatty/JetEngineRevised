// source/game/online/savegame/SaveVerifierMgr.cpp
#include "SaveVerifierMgr.h"
#include <cstdio>
#include <random>
#include "stream/memorystream.h"
#include "stream/memlinkstream.h"
#include "system.h"

const jet::String SaveVerifierMgr::k_recordName("SaveVerifierMgr");

// The GLUID of the device: three values and the algorithm that made them (GLUID::GetGenerationAlgorithm), made from
// random ones (the .so hashes the hardware id) when the device has none yet.
static jet::String LoadOrMakeGLUID() {
    std::string path = std::string(jet::System::GetUserFolder().c_str()) + "/gluid_enc";
    unsigned v[4] = { 0, 0, 0, 0 };
    bool found = false;
    if (FILE *f = fopen(path.c_str(), "rb")) {
        found = fread(v, sizeof(v), 1, f) == 1;
        fclose(f);
    }
    if (!found) {
        std::random_device rd;
        for (int i = 0; i < 3; i++) v[i] = rd() ^ ((unsigned)jet::System::GetUTime() * 2654435761u);
        v[3] = 1;
        if (FILE *f = fopen(path.c_str(), "wb")) {
            fwrite(v, sizeof(v), 1, f);
            fclose(f);
        }
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%u %u %u %u", v[0], v[1], v[2], v[3]);
    return jet::String(buf);
}

jet::String SaveVerifierMgr::RetrieveEncryptionGLUID() {
    static jet::String s_gluid = LoadOrMakeGLUID();
    return s_gluid;
}

// @0x4f9d74: the record is a binary one with the GLUID written as a string
void SaveVerifierMgr::WriteValidSave(clara::RecordDB &db) {
    jet::stream::MemoryStream ms;
    ms.Start();
    jet::String gluid = RetrieveEncryptionGLUID();
    jet::WriteString(ms, gluid);
    ms.SeekBeg(0);
    clara::Record r(clara::Record::TypeDef::BINARY);
    if (ms.Size()) r.Set(ms.GetInternalArray().data(), ms.Size());
    db.Set(k_recordName, r);
    ms.Finish();
}

// @0x4f9ea0: a save with no record, or an empty one, reads as the empty string and is not the device's
bool SaveVerifierMgr::IsValidSave(clara::RecordDB &db) {
    jet::String saved;
    const clara::Record &r = db.Get(k_recordName);
    if (r.GetType() != clara::Record::TypeDef::NONE && !r.GetAsBinary().empty()) {
        jet::stream::MemLinkStream s(r.GetAsBinary());
        s.Start();
        s >> saved;
        s.Finish();
    }
    return saved.Equals(RetrieveEncryptionGLUID());
}
