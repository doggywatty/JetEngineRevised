// libs/vox1.1/src/vox_soundpack_xml.cpp
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <pugixml.hpp>
#include "vox_soundpack_xml.h"

#ifdef _WIN32
// the win32 C runtime has no lrand48; two rand() draws give the same 31-bit range for these picks
static long lrand48() {
    return ((long)rand() << 16) ^ (long)rand();
}
#endif

namespace vox {

static std::string Lower(const char *s) {
    std::string r(s ? s : "");
    for (size_t i = 0; i < r.size(); i++) r[i] = (char)tolower((unsigned char)r[i]);
    return r;
}

static bool EqualsNoCase(const std::string &a, const char *b) { return Lower(a.c_str()) == Lower(b); }

// the gains of the file are in dB, the pitches in cents (@0x5b6ff0: pow(10, dB / 20), @0x5b7284: pow(2, cents / 1200))
static float DbToLinear(float db) { return (float)pow(10.0, db / 20.0); }
static float CentsToRatio(float cents) { return (float)pow(2.0, cents / 1200.0); }

// the seconds of the clock that the cooldown of the events counts in
static double NowSeconds() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

GroupXMLDef::GroupXMLDef()
    : m_is3D(k3DOff), m_parentId((unsigned)-1), m_volume(1.0f), m_enable(true), m_refdistance(1.0f), m_maxdistance(1000000.0f), m_rolloff(1.0f),
      m_baseGain(1.0f), m_basePitch(1.0f), m_maxGainMod(1.0f), m_maxPitchMod(0.0f), m_minGainMod(1.0f), m_minPitchMod(0.0f), m_killOnResume(false),
      m_fadeOnPlay(0.0f), m_fadeOnStop(0.0f) {}

BankXMLDef::BankXMLDef() : m_threshold((int)0x80000001), m_maxPlayback(0), m_behaviour(priority_bank::kDoNothing) {}

SoundXMLDef::SoundXMLDef()
    : m_priority(0), m_loadingFlags(kLoadNone), m_format(kFormatNone), m_bankId(-1), m_groupId(-1), m_isLoop(false), m_is3D(k3DOff), m_referenceDistance(1.0f),
      m_maxDistance(1000000.0f), m_rolloffFactor(1.0f), m_baseGain(1.0f), m_minGainMod(1.0f), m_maxGainMod(1.0f), m_isGainRandom(false), m_basePitch(1.0f),
      m_minPitchMod(0.0f), m_maxPitchMod(0.0f), m_isPitchRandom(false), m_killOnResume(false), m_fadeOnPlay(0.0f), m_fadeOnStop(0.0f) {}

EventXMLDef::EventXMLDef()
    : m_type(0), m_noRepeatLength(0), m_params1(0), m_cooldownValue(0.0f), m_cooldownType(0), m_lastPlayTime(-999999.0), m_cooldownRemaining(0.0), m_position(0) {}

VoxSoundPackXML::VoxSoundPackXML() : data(new VoxSoundPackXMLInternalData()) {}

VoxSoundPackXML::VoxSoundPackXML(const char *path) : data(new VoxSoundPackXMLInternalData()) { LoadXML(path); }

VoxSoundPackXML::~VoxSoundPackXML() { delete data; }

int VoxSoundPackXML::GetSoundCount() { return (int)data->m_soundVector.size(); }
int VoxSoundPackXML::GetBankCount() { return (int)data->m_bankVector.size(); }
int VoxSoundPackXML::GetGroupCount() { return (int)data->m_groupVector.size(); }
int VoxSoundPackXML::GetEventCount() { return (int)data->m_eventVector.size(); }

bool VoxSoundPackXML::GetConfigInfo(ConfigInfoXML &info) {
    info = data->m_config;
    return true;
}

static bool AttrBool(const pugi::xml_node &n, const char *name, bool &out, bool yesIsFirstLetter) {
    pugi::xml_attribute a = n.attribute(name);
    if (!a) return false;
    const char *v = a.value();
    // @0x5b7048: "enable" and "killonresume" are on unless the value starts with 'n'; "loop" is on when it starts with 'y'
    out = yesIsFirstLetter ? (toupper((unsigned char)v[0]) == 'Y') : (v[0] != 'n');
    return true;
}

static Vox3DSoundType Mode3D(const char *v) { return v && *v == 'y' ? k3DWorld : v && *v == 'r' ? k3DRelative : k3DOff; }

static const char *ExtensionOf(int format) {
    switch (format) {
        case kFormatWav: return ".wav";
        case kFormatMpc: return ".mpc";
        case kFormatOgg: return ".ogg";
        case kFormatVxn: return ".vxn";
        case kFormatBcwav: return ".bcwav";
        default: return "";
    }
}

static int FormatOf(const char *v) {
    if (!strcmp(v, "pcm") || !strcmp(v, "adpcm")) return kFormatWav;
    if (!strcmp(v, "mpc8") || !strcmp(v, "mpc")) return kFormatMpc;
    if (!strcmp(v, "ogg")) return kFormatOgg;
    if (!strcmp(v, "vxn")) return kFormatVxn;
    if (!strcmp(v, "bcwav")) return kFormatBcwav;
    return kFormatNone;
}

// @0x5b66e4
bool VoxSoundPackXML::LoadXML(const char *path) {
    FileInterface *f = FileInterface::Open(path, kAccessRead);
    if (!f) return false;
    std::vector<char> buf;
    char chunk[16384];
    for (;;) {
        const int n = f->Read(chunk, 1, sizeof(chunk));
        if (n <= 0) break;
        buf.insert(buf.end(), chunk, chunk + n);
    }
    delete f;
    if (buf.empty()) return false;
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_buffer(&buf[0], buf.size());
    if (!res) return false;
    pugi::xml_node root = doc.child("soundpack");
    if (!root) return false;

    VoxSoundPackXMLInternalData &d = *data;
    d = VoxSoundPackXMLInternalData();
    if (pugi::xml_node c = root.child("config")) {
        d.m_config.apiVersion = c.attribute("apiversion").value();
        d.m_config.console = c.attribute("console").value();
        d.m_config.nameOfGame = c.attribute("nameofgame").value();
        d.m_config.descriptorVersion = c.attribute("descriptorversion").value();
    }

    // the groups (@0x5b6cf0)
    for (pugi::xml_node g = root.child("groups").child("group"); g; g = g.next_sibling()) {
        GroupXMLDef def;
        def.m_name = g.attribute("name").value();
        def.m_busName = g.attribute("bus").value();
        if (pugi::xml_attribute a = g.attribute("mode3d")) def.m_is3D = Mode3D(a.value());
        if (pugi::xml_attribute a = g.attribute("parent")) {
            // a parent is a group of the file, by its name; one that is not there is the first
            def.m_parentId = 0;
            for (size_t i = 0; i < d.m_groupVector.size(); i++)
                if (EqualsNoCase(d.m_groupVector[i].m_name, a.value())) { def.m_parentId = (unsigned)i; break; }
        }
        if (pugi::xml_attribute a = g.attribute("gain")) def.m_volume = DbToLinear(a.as_float());
        AttrBool(g, "enable", def.m_enable, false);
        if (pugi::xml_attribute a = g.attribute("refdistance")) def.m_refdistance = a.as_float();
        if (pugi::xml_attribute a = g.attribute("maxdistance")) def.m_maxdistance = a.as_float();
        if (pugi::xml_attribute a = g.attribute("rolloff")) def.m_rolloff = a.as_float();
        if (pugi::xml_attribute a = g.attribute("basegain")) def.m_baseGain = DbToLinear(a.as_float());
        if (pugi::xml_attribute a = g.attribute("mingainmod")) def.m_minGainMod = DbToLinear(a.as_float());
        if (pugi::xml_attribute a = g.attribute("maxgainmod")) def.m_maxGainMod = DbToLinear(a.as_float());
        if (pugi::xml_attribute a = g.attribute("basepitch")) def.m_basePitch = CentsToRatio(a.as_float());
        if (pugi::xml_attribute a = g.attribute("minpitchmod")) def.m_minPitchMod = a.as_float();
        if (pugi::xml_attribute a = g.attribute("maxpitchmod")) def.m_maxPitchMod = a.as_float();
        AttrBool(g, "killonresume", def.m_killOnResume, false);
        if (pugi::xml_attribute a = g.attribute("fadeonplay")) def.m_fadeOnPlay = a.as_float();
        if (pugi::xml_attribute a = g.attribute("fadeonstop")) def.m_fadeOnStop = a.as_float();
        d.m_groupVector.push_back(def);
    }

    // the banks
    for (pugi::xml_node b = root.child("banks").child("bank"); b; b = b.next_sibling()) {
        BankXMLDef def;
        def.m_name = b.attribute("label").value();
        if (pugi::xml_attribute a = b.attribute("maxplaybacks")) def.m_maxPlayback = (unsigned)a.as_int();
        if (pugi::xml_attribute a = b.attribute("threshold")) def.m_threshold = a.as_int();
        if (pugi::xml_attribute a = b.attribute("behaviour")) {
            const char *v = a.value();
            if (!strcmp(v, "steal oldest")) def.m_behaviour = priority_bank::kStealOldest;
            else if (!strcmp(v, "steal lowest priority")) def.m_behaviour = priority_bank::kStealLowestPriority;
            else if (!strcmp(v, "steal low. prio. or old. same prio")) def.m_behaviour = priority_bank::kStealLowPriorityOrOldestSamePriority;
            else if (!strcmp(v, "steal quietest")) def.m_behaviour = priority_bank::kStealQuietest;
        }
        d.m_bankVector.push_back(def);
    }

    // the sounds (@0x5b77e0): what a sound does not say is the default of its group
    for (pugi::xml_node s = root.child("sounds").child("sound"); s; s = s.next_sibling()) {
        SoundXMLDef def;
        def.m_label = s.attribute("label").value();
        for (size_t i = 0; i < d.m_bankVector.size(); i++)
            if (EqualsNoCase(d.m_bankVector[i].m_name, s.attribute("bank").value())) { def.m_bankId = (int)i; break; }
        for (size_t i = 0; i < d.m_groupVector.size(); i++)
            if (EqualsNoCase(d.m_groupVector[i].m_name, s.attribute("group").value())) { def.m_groupId = (int)i; break; }
        const GroupXMLDef *g = def.m_groupId >= 0 ? &d.m_groupVector[def.m_groupId] : 0;
        if (g) {
            def.m_is3D = g->m_is3D;
            def.m_referenceDistance = g->m_refdistance;
            def.m_maxDistance = g->m_maxdistance;
            def.m_rolloffFactor = g->m_rolloff;
            def.m_baseGain = g->m_baseGain;
            def.m_minGainMod = g->m_minGainMod;
            def.m_maxGainMod = g->m_maxGainMod;
            def.m_basePitch = g->m_basePitch;
            def.m_minPitchMod = g->m_minPitchMod;
            def.m_maxPitchMod = g->m_maxPitchMod;
            def.m_killOnResume = g->m_killOnResume;
            def.m_fadeOnPlay = g->m_fadeOnPlay;
            def.m_fadeOnStop = g->m_fadeOnStop;
            def.m_bus = g->m_busName;
        }
        if (pugi::xml_attribute a = s.attribute("priority")) def.m_priority = a.as_int();
        AttrBool(s, "loop", def.m_isLoop, true);
        if (pugi::xml_attribute a = s.attribute("format")) def.m_format = (signed char)FormatOf(a.value());
        if (pugi::xml_attribute a = s.attribute("loadingflags")) {
            const char *v = a.value();
            if (!strcmp(v, "none")) def.m_loadingFlags = kLoadNone;
            else if (!strcmp(v, "load to ram")) def.m_loadingFlags = kLoadToRam;
            else if (!strcmp(v, "load and decode")) def.m_loadingFlags = kLoadAndDecode;
        }
        // the file is the label with the extension of its format unless the sound names it
        if (pugi::xml_attribute a = s.attribute("filename")) def.m_filename = a.value();
        else def.m_filename = def.m_label + ExtensionOf(def.m_format);
        if (pugi::xml_attribute a = s.attribute("bus")) def.m_bus = a.value();
        if (pugi::xml_attribute a = s.attribute("mode3d")) def.m_is3D = Mode3D(a.value());
        if (pugi::xml_attribute a = s.attribute("refdistance")) def.m_referenceDistance = a.as_float();
        if (pugi::xml_attribute a = s.attribute("maxdistance")) def.m_maxDistance = a.as_float();
        if (pugi::xml_attribute a = s.attribute("rolloff")) def.m_rolloffFactor = a.as_float();
        if (pugi::xml_attribute a = s.attribute("basegain")) def.m_baseGain = DbToLinear(a.as_float());
        if (pugi::xml_attribute a = s.attribute("mingainmod")) { def.m_minGainMod = DbToLinear(a.as_float()); def.m_isGainRandom = true; }
        if (pugi::xml_attribute a = s.attribute("maxgainmod")) { def.m_maxGainMod = DbToLinear(a.as_float()); def.m_isGainRandom = true; }
        if (pugi::xml_attribute a = s.attribute("basepitch")) def.m_basePitch = CentsToRatio(a.as_float());
        if (pugi::xml_attribute a = s.attribute("minpitchmod")) { def.m_minPitchMod = a.as_float(); def.m_isPitchRandom = true; }
        if (pugi::xml_attribute a = s.attribute("maxpitchmod")) { def.m_maxPitchMod = a.as_float(); def.m_isPitchRandom = true; }
        AttrBool(s, "killonresume", def.m_killOnResume, false);
        if (pugi::xml_attribute a = s.attribute("fadeonplay")) def.m_fadeOnPlay = a.as_float();
        if (pugi::xml_attribute a = s.attribute("fadeonstop")) def.m_fadeOnStop = a.as_float();
        d.m_soundsHashes[Lower(def.m_label.c_str())] = (int)d.m_soundVector.size();
        d.m_soundVector.push_back(def);
    }

    // the events (@0x5b87e8): "value" is the labels of the sounds, "params" the length of the memory of a random event
    for (pugi::xml_node e = root.child("events").child("event"); e; e = e.next_sibling()) {
        EventXMLDef def;
        def.m_label = e.attribute("label").value();
        if (pugi::xml_attribute a = e.attribute("type")) {
            const char *v = a.value();
            def.m_type = !strcmp(v, "random") ? 0 : !strcmp(v, "playlist") ? 1 : !strcmp(v, "pl_random") ? 2 : 0;
        }
        if (pugi::xml_attribute a = e.attribute("value")) {
            std::string list = a.value();
            size_t pos = 0;
            while (pos < list.size()) {
                size_t end = list.find_first_of(" ,", pos);
                if (end == std::string::npos) end = list.size();
                if (end > pos) {
                    std::map<std::string, int>::iterator it = d.m_soundsHashes.find(Lower(list.substr(pos, end - pos).c_str()));
                    if (it != d.m_soundsHashes.end()) def.m_sounds.push_back(it->second);
                }
                pos = end + 1;
            }
        }
        if (pugi::xml_attribute a = e.attribute("params")) {
            std::string list = a.value();
            const int first = atoi(list.c_str());
            def.m_noRepeatLength = (unsigned short)std::min<int>(first, (int)def.m_sounds.size());
            const size_t sep = list.find_first_of(" ;");
            if (sep != std::string::npos) def.m_params1 = (unsigned short)std::min(atoi(list.c_str() + sep + 1), 100);
        }
        if (pugi::xml_attribute a = e.attribute("cooldownvalue")) def.m_cooldownValue = a.as_float();
        if (pugi::xml_attribute a = e.attribute("cooldowntype")) def.m_cooldownType = !strcmp(a.value(), "plays") ? 1 : 0;
        d.m_eventsHashes[Lower(def.m_label.c_str())] = (int)d.m_eventVector.size();
        d.m_eventVector.push_back(def);
        ResetEvent((int)d.m_eventVector.size() - 1);
    }
    return true;
}

unsigned VoxSoundPackXML::GetBankUid(const char *name) {
    for (size_t i = 0; i < data->m_bankVector.size(); i++)
        if (EqualsNoCase(data->m_bankVector[i].m_name, name)) return (unsigned)i;
    return (unsigned)-1;
}

bool VoxSoundPackXML::GetBankInfo(unsigned uid, priority_bank::CreationSettings &out) {
    if (uid >= data->m_bankVector.size()) return false;
    const BankXMLDef &b = data->m_bankVector[uid];
    out.m_name = b.m_name.c_str();
    out.m_threshold = b.m_threshold;
    out.m_maxPlayback = b.m_maxPlayback;
    out.m_behaviour = b.m_behaviour;
    return true;
}

bool VoxSoundPackXML::GetBankInfo(const char *name, priority_bank::CreationSettings &out) { return GetBankInfo(GetBankUid(name), out); }

unsigned VoxSoundPackXML::GetGroupUid(const char *name) {
    for (size_t i = 0; i < data->m_groupVector.size(); i++)
        if (EqualsNoCase(data->m_groupVector[i].m_name, name)) return (unsigned)i;
    return (unsigned)-1;
}

bool VoxSoundPackXML::GetGroupInfo(unsigned uid, group::CreationSettings &out) {
    if (uid >= data->m_groupVector.size()) return false;
    const GroupXMLDef &g = data->m_groupVector[uid];
    out.m_name = g.m_name.c_str();
    out.m_busName = g.m_busName.c_str();
    out.m_parentId = g.m_parentId;
    out.m_volume = g.m_volume;
    out.m_pitch = 1.0f;
    out.m_enable = g.m_enable;
    out.m_killOnResume = g.m_killOnResume;
    out.m_fadeOnPlay = g.m_fadeOnPlay;
    out.m_fadeOnStop = g.m_fadeOnStop;
    return true;
}

bool VoxSoundPackXML::GetGroupInfo(const char *name, group::CreationSettings &out) { return GetGroupInfo(GetGroupUid(name), out); }

int VoxSoundPackXML::GetSoundUid(const char *label) {
    if (!label) return -1;
    std::map<std::string, int>::iterator it = data->m_soundsHashes.find(Lower(label));
    return it == data->m_soundsHashes.end() ? -1 : it->second;
}

bool VoxSoundPackXML::GetSoundLabel(int uid, const char *&label) {
    if (uid < 0 || uid >= (int)data->m_soundVector.size()) return false;
    label = data->m_soundVector[uid].m_label.c_str();
    return true;
}

// the stream of a sound is a file (type 0) of the name the soundpack gives, and its decoder is of its format
bool VoxSoundPackXML::GetDataSourceInfo(int uid, data_source::CreationSettings &out) {
    if (uid < 0 || uid >= (int)data->m_soundVector.size()) return false;
    const SoundXMLDef &s = data->m_soundVector[uid];
    out.m_streamType = kStreamCFile;
    out.m_pStreamParams = (void*)s.m_filename.c_str();
    out.m_decoderType = s.m_format;
    out.m_pDecoderParams = 0;
    out.m_loadingFlags = s.m_loadingFlags;
    out.m_uid = uid;
    out.m_groupId = (unsigned)s.m_groupId;
    out.m_numCustomSound = 0;
    out.m_customSoundStr = 0;
    return true;
}

bool VoxSoundPackXML::GetDataSourceInfo(const char *label, data_source::CreationSettings &out) { return GetDataSourceInfo(GetSoundUid(label), out); }

// @0x5b93b0
bool VoxSoundPackXML::GetEmitterInfo(int uid, emitter::CreationSettings &out) {
    if (uid < 0 || uid >= (int)data->m_soundVector.size()) return false;
    const SoundXMLDef &s = data->m_soundVector[uid];
    out = emitter::CreationSettings(s.m_baseGain, s.m_basePitch, (unsigned)s.m_groupId, s.m_priority, (unsigned)s.m_bankId);
    out.SetRandomGain(s.m_minGainMod, s.m_maxGainMod, s.m_isGainRandom);
    out.SetRandomPitch(s.m_minPitchMod, s.m_maxPitchMod, s.m_isPitchRandom);
    out.m_isLoop = s.m_isLoop;
    out.m_is3d = s.m_is3D != k3DOff;
    out.m_pEmitter3DParameters.relativeToListener = s.m_is3D == k3DRelative ? 1 : 0;
    out.m_pEmitter3DParameters.referenceDistance = s.m_referenceDistance;
    out.m_pEmitter3DParameters.maxDistance = s.m_maxDistance;
    out.m_pEmitter3DParameters.rolloffFactor = s.m_rolloffFactor;
    out.m_busName = s.m_bus.c_str();
    out.m_killOnResume = s.m_killOnResume;
    out.m_fadeOnPlay = s.m_fadeOnPlay;
    out.m_fadeOnStop = s.m_fadeOnStop;
    out.m_uid = uid;
    return true;
}

bool VoxSoundPackXML::GetEmitterInfo(const char *label, emitter::CreationSettings &out) { return GetEmitterInfo(GetSoundUid(label), out); }

// @0x5b9620: a label is a sound, or an event that gives one of its sounds
bool VoxSoundPackXML::GetEmitterInfoFromSoundOrEvent(const char *label, emitter::CreationSettings &out) {
    if (!data) return false;
    if (GetEmitterInfo(label, out)) return true;
    int uid = -1;
    if (!GetEventSoundUid(label, uid)) return false;
    return GetEmitterInfo(uid, out);
}

bool VoxSoundPackXML::GetSoundCustomParam(int, int, const char *&) { return false; }

bool VoxSoundPackXML::GetSoundCustomParam(const char *label, int index, const char *&value) { return GetSoundCustomParam(GetSoundUid(label), index, value); }

int VoxSoundPackXML::GetEventUid(const char *label) {
    if (!label) return -1;
    std::map<std::string, int>::iterator it = data->m_eventsHashes.find(Lower(label));
    return it == data->m_eventsHashes.end() ? -1 : it->second;
}

int VoxSoundPackXML::GetEventSize(int uid) { return uid >= 0 && uid < (int)data->m_eventVector.size() ? (int)data->m_eventVector[uid].m_sounds.size() : 0; }

int VoxSoundPackXML::GetEventSize(const char *label) { return GetEventSize(GetEventUid(label)); }

bool VoxSoundPackXML::GetEventCustomParam(int, int, const char *&) { return false; }

bool VoxSoundPackXML::GetEventCustomParam(const char *label, int index, const char *&value) { return GetEventCustomParam(GetEventUid(label), index, value); }

// @0x5b9c30: the event starts again: a playlist from the beginning, a "pl_random" one from a random sound, no cooldown
bool VoxSoundPackXML::ResetEvent(int uid) {
    if (uid < 0 || uid >= (int)data->m_eventVector.size()) return false;
    EventXMLDef &e = data->m_eventVector[uid];
    const int n = (int)e.m_sounds.size();
    e.m_position = e.m_type == 2 && n > 0 ? (int)(lrand48() % n) : n;
    e.m_recent.clear();
    e.m_lastPlayTime = -999999.0;
    e.m_cooldownRemaining = 0.0;
    return true;
}

bool VoxSoundPackXML::ResetEvent(const char *label) { return ResetEvent(GetEventUid(label)); }

// @0x5b990c: the sound an event plays now, false when its cooldown is not over
bool VoxSoundPackXML::GetEventSoundUid(int uid, int &soundUid) {
    if (uid < 0 || uid >= (int)data->m_eventVector.size()) return false;
    EventXMLDef &e = data->m_eventVector[uid];
    const int n = (int)e.m_sounds.size();
    if (n == 0) return false;
    const double now = NowSeconds();
    if (e.m_cooldownValue > 0.0f) {
        if (e.m_cooldownType == 0) {
            if (now - e.m_lastPlayTime < e.m_cooldownValue) return false;
        } else if (e.m_cooldownRemaining >= 1.0) {
            // "plays": the event is let by again after that many tries
            e.m_cooldownRemaining -= 1.0;
            return false;
        }
    }
    int index = 0;
    if (e.m_type == 0) {
        // at random, not one of the last m_noRepeatLength
        std::vector<int> candidates;
        for (int i = 0; i < n; i++)
            if (std::find(e.m_recent.begin(), e.m_recent.end(), i) == e.m_recent.end()) candidates.push_back(i);
        index = candidates.empty() ? (int)(lrand48() % n) : candidates[lrand48() % candidates.size()];
        e.m_recent.push_back(index);
        while ((int)e.m_recent.size() > std::min<int>(e.m_noRepeatLength, n - 1)) e.m_recent.erase(e.m_recent.begin());
    } else {
        // in turn (from the first, or from the random one a "pl_random" event started at)
        index = e.m_position >= n ? 0 : e.m_position;
        e.m_position = index + 1;
    }
    e.m_lastPlayTime = now;
    if (e.m_cooldownType == 1) e.m_cooldownRemaining = e.m_cooldownValue;
    soundUid = e.m_sounds[index];
    return true;
}

bool VoxSoundPackXML::GetEventSoundUid(const char *label, int &soundUid) { return GetEventSoundUid(GetEventUid(label), soundUid); }

// @0x5b9ea4: the groups of the file are the groups of the engine, in its order
bool VoxSoundPackXML::AutoSetupGroups() {
    VoxEngine &engine = VoxEngine::GetVoxEngine();
    for (size_t i = 0; i < data->m_groupVector.size(); i++) {
        group::CreationSettings s;
        if (!GetGroupInfo((unsigned)i, s)) return false;
        engine.AddGroup(s);
    }
    return true;
}

// @0x5b9f68
bool VoxSoundPackXML::AutoSetupBanks() {
    VoxEngine &engine = VoxEngine::GetVoxEngine();
    for (size_t i = 0; i < data->m_bankVector.size(); i++) {
        priority_bank::CreationSettings s;
        if (!GetBankInfo((unsigned)i, s)) return false;
        engine.AddPriorityBank(s);
    }
    return true;
}

}  // namespace vox
