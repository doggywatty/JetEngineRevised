// libs/vox1.1/include/vox_soundpack_xml.h
// The soundpack: the XML that tells what the sounds of the game are (label, file, format, group, bank, priority, gains, 3D),
// the groups, the priority banks and the events (a label that plays one of some sounds). LoadXML is @0x5b66e4 in the .so
// (vox_soundpack_xml.cpp @0x5b5b00..0x5bb800: the loaders of the groups, banks, sounds and events are in it).
#ifndef LIBS_VOX1_1_INCLUDE_VOX_SOUNDPACK_XML_H
#define LIBS_VOX1_1_INCLUDE_VOX_SOUNDPACK_XML_H

#include <map>
#include <string>
#include <vector>
#include "vox.h"

namespace vox {

struct ConfigInfoXML {
    std::string apiVersion;
    std::string console;
    std::string nameOfGame;
    std::string descriptorVersion;
};

struct GroupXMLDef {  // sizeof 0x48
    GroupXMLDef();
    std::string m_busName;  // 0x0
    std::string m_name;  // 0x4
    Vox3DSoundType m_is3D;  // 0x8
    unsigned m_parentId;  // 0xC
    float m_volume;  // 0x10
    bool m_enable;  // 0x14
    float m_refdistance;  // 0x18
    float m_maxdistance;  // 0x1C
    float m_rolloff;  // 0x20
    float m_baseGain;  // 0x24
    float m_basePitch;  // 0x28
    float m_maxGainMod;  // 0x2C
    float m_maxPitchMod;  // 0x30
    float m_minGainMod;  // 0x34
    float m_minPitchMod;  // 0x38
    bool m_killOnResume;  // 0x3C
    float m_fadeOnPlay;  // 0x40
    float m_fadeOnStop;  // 0x44
};

struct BankXMLDef {
    BankXMLDef();
    std::string m_name;
    int m_threshold;
    unsigned m_maxPlayback;
    priority_bank::Behaviour m_behaviour;
};

struct SoundXMLDef {  // sizeof 0x58
    SoundXMLDef();
    int m_priority;  // 0x0
    std::string m_label;  // 0x4
    std::string m_filename;  // 0x8
    VoxSourceLoadingFlags m_loadingFlags;  // 0xC
    signed char m_format;  // 0x10
    int m_bankId;  // 0x11
    int m_groupId;  // 0x12
    bool m_isLoop;  // 0x13
    std::string m_bus;  // 0x14
    Vox3DSoundType m_is3D;  // 0x18
    float m_referenceDistance;  // 0x1C
    float m_maxDistance;  // 0x20
    float m_rolloffFactor;  // 0x24
    float m_baseGain;  // 0x28
    float m_minGainMod;  // 0x2C
    float m_maxGainMod;  // 0x30
    bool m_isGainRandom;  // 0x34
    float m_basePitch;  // 0x38
    float m_minPitchMod;  // 0x3C
    float m_maxPitchMod;  // 0x40
    bool m_isPitchRandom;  // 0x44
    bool m_killOnResume;  // 0x45
    float m_fadeOnPlay;  // 0x48
    float m_fadeOnStop;  // 0x4C
};

// an event: a label that plays one of some sounds, picked at random ("random"), in turn ("playlist") or in turn from a random
// one ("pl_random"); a cooldown keeps it from playing again at once (in seconds, or in plays)
struct EventXMLDef {  // sizeof 0x38
    EventXMLDef();
    std::string m_label;
    int m_type;  // 0x18: 0 random, 1 playlist, 2 pl_random
    std::vector<int> m_sounds;  // 0xC: the uids of the sounds
    unsigned short m_noRepeatLength;  // 0x1A: the last sounds a random event does not pick again
    unsigned short m_params1;  // 0x1C
    float m_cooldownValue;  // 0x20
    int m_cooldownType;  // 0x24: 0 time, 1 plays
    double m_lastPlayTime;  // 0x28 (the cooldown)
    double m_cooldownRemaining;
    int m_position;  // 0x1E: the next sound of a playlist
    std::vector<int> m_recent;
};

class VoxSoundPackXMLInternalData {  // sizeof 0x98
public:
    VoxSoundPackXMLInternalData() {}
    ConfigInfoXML m_config;  // 0x0
    std::vector<SoundXMLDef> m_soundVector;  // 0x18
    std::vector<GroupXMLDef> m_groupVector;  // 0x2C
    std::vector<BankXMLDef> m_bankVector;  // 0x40
    std::vector<EventXMLDef> m_eventVector;  // 0x54
    std::map<std::string, int> m_soundsHashes;  // 0x7C: the lower case label to the uid
    std::map<std::string, int> m_eventsHashes;  // 0x68
    // the stream parameters of the sounds: the path that a data source's stream reads
    std::vector<std::string> m_paths;
};

class VoxSoundPackXML {  // sizeof 0x4
public:
    explicit VoxSoundPackXML(const char *path);
    VoxSoundPackXML();
    ~VoxSoundPackXML();
    int GetSoundCount();
    int GetBankCount();
    int GetGroupCount();
    int GetEventCount();
    bool GetConfigInfo(ConfigInfoXML &info);
    unsigned GetBankUid(const char *name);
    bool GetBankInfo(unsigned uid, priority_bank::CreationSettings &out);
    bool GetBankInfo(const char *name, priority_bank::CreationSettings &out);
    unsigned GetGroupUid(const char *name);
    bool GetGroupInfo(unsigned uid, group::CreationSettings &out);
    bool GetGroupInfo(const char *name, group::CreationSettings &out);
    // @0x5b91ec: the uid of a sound by its label, -1 when there is none
    int GetSoundUid(const char *label);
    bool GetSoundLabel(int uid, const char *&label);
    // @0x5b9298
    bool GetDataSourceInfo(int uid, data_source::CreationSettings &out);
    bool GetDataSourceInfo(const char *label, data_source::CreationSettings &out);
    // @0x5b93b0
    bool GetEmitterInfo(int uid, emitter::CreationSettings &out);
    bool GetEmitterInfo(const char *label, emitter::CreationSettings &out);
    // @0x5b9620: a sound's, or the one of an event's that plays now
    bool GetEmitterInfoFromSoundOrEvent(const char *label, emitter::CreationSettings &out);
    bool GetSoundCustomParam(int uid, int index, const char *&value);
    bool GetSoundCustomParam(const char *label, int index, const char *&value);
    int GetEventUid(const char *label);
    // the sound of an event that plays now (@0x5b990c), and the reset of an event (@0x5b9c30)
    bool GetEventSoundUid(int eventUid, int &soundUid);
    bool GetEventSoundUid(const char *label, int &soundUid);
    bool ResetEvent(int eventUid);
    bool ResetEvent(const char *label);
    int GetEventSize(int uid);
    int GetEventSize(const char *label);
    bool GetEventCustomParam(int uid, int index, const char *&value);
    bool GetEventCustomParam(const char *label, int index, const char *&value);
    // @0x5b66e4: the file is read through the file system of the engine
    bool LoadXML(const char *path);
    // @0x5b9ea4 / @0x5b9f68: the groups and the banks of the soundpack are made in the engine, in the order of the file
    bool AutoSetupGroups();
    bool AutoSetupBanks();
private:
    VoxSoundPackXMLInternalData *data;  // 0x0
};

}  // namespace vox

#endif
