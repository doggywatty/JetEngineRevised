// source/game/gamemanagers/COPPAMgr.cpp
#include "COPPAMgr.h"
#include <strings.h>
#include <string>
#include <vector>

namespace {
enum Kind { COPPA, OFT, Italy, AUNZ };
// k_countryInfoDB (82 entries, @0x558990 static init); a country not in it gets the Italian rules (GetCountryInfo)
const struct { const char *code; Kind kind; } k_countries[] = {
    {"US", COPPA}, {"VI", COPPA}, {"UM", COPPA}, {"AS", COPPA}, {"GU", COPPA}, {"MP", COPPA}, {"PR", COPPA}, {"CA", COPPA},
    {"GB", OFT}, {"AI", OFT}, {"BM", OFT}, {"GI", OFT}, {"GS", OFT}, {"KY", OFT}, {"FK", OFT}, {"PN", OFT}, {"TC", OFT}, {"VG", OFT},
    {"MS", OFT}, {"SH", OFT}, {"IO", OFT}, {"GG", OFT}, {"IM", OFT}, {"JE", OFT}, {"DG", OFT},
    {"NZ", AUNZ}, {"CK", AUNZ}, {"NU", AUNZ}, {"TK", AUNZ}, {"AU", AUNZ}, {"CX", AUNZ}, {"NF", AUNZ}, {"CC", AUNZ}, {"HM", AUNZ},
    {"DE", OFT}, {"AT", OFT}, {"BE", OFT}, {"BG", OFT}, {"CY", OFT}, {"HR", OFT}, {"DK", OFT}, {"GL", OFT}, {"ES", OFT}, {"IC", OFT},
    {"EA", OFT}, {"EE", OFT}, {"FI", OFT}, {"AX", OFT}, {"FR", OFT}, {"GF", OFT}, {"PF", OFT}, {"GP", OFT}, {"MQ", OFT}, {"YT", OFT},
    {"NC", OFT}, {"RE", OFT}, {"BL", OFT}, {"MF", OFT}, {"PM", OFT}, {"WF", OFT}, {"GR", OFT}, {"HU", OFT}, {"IE", OFT}, {"IT", Italy},
    {"LV", OFT}, {"LT", OFT}, {"LU", OFT}, {"MT", OFT}, {"NL", OFT}, {"AW", OFT}, {"CW", OFT}, {"BQ", OFT}, {"SX", OFT}, {"PL", OFT},
    {"PT", OFT}, {"CZ", OFT}, {"RO", OFT}, {"SK", OFT}, {"SI", OFT}, {"SE", OFT}, {"FO", OFT}, {"TF", OFT},
};
const unsigned k_countryInfoDBSize = sizeof(k_countries) / sizeof(k_countries[0]);
const unsigned k_fallbackCountry = 63;  // GetCountryInfo returns the entry at 0x4ec: Italy

// CountryInfo::MakeCOPPA @0x545e1c, MakeOFT @0x545e78, MakeItaly @0x545ee0, MakeAUNZ @0x545f00
std::vector<COPPAMgr::CountryInfo> MakeCountryInfoDB() {
    std::vector<COPPAMgr::CountryInfo> db(k_countryInfoDBSize);
    for (unsigned i = 0; i < k_countryInfoDBSize; i++) {
        COPPAMgr::CountryInfo &c = db[i];
        c.code = k_countries[i].code;
        c.oft = c.oftD = c.oftE = c.italy = c.oft10 = c.auNz = c.coppa = false;
        if (k_countries[i].kind == COPPA) {
            c.minimumAge = 13;
            c.coppa = true;
            continue;
        }
        c.minimumAge = 16;
        c.oft10 = c.oft = c.oftD = c.oftE = true;
        if (k_countries[i].kind == Italy) c.italy = c.oft10 = true;
        if (k_countries[i].kind == AUNZ) {
            c.minimumAge = 13;
            c.auNz = true;
        }
    }
    return db;
}
const std::vector<COPPAMgr::CountryInfo> &CountryInfoDB() {
    static const std::vector<COPPAMgr::CountryInfo> db = MakeCountryInfoDB();
    return db;
}
}

const jet::String COPPAMgr::k_saveRecordName("COPPAMgr");

// @0x545f60: the save data reset (COPPAMgr::ResetData @0x546094)
COPPAMgr::COPPAMgr() { ResetData(m_data); }

// @0x546094
void COPPAMgr::ResetData(COPPAMgrSaveData &d) const {
    d.flags = 0;
    d.prevFlags = 0;
    d.lastEmailQuestionTime = 0;
}

// @0x5460b0
void COPPAMgr::SaveDataInStream(jet::stream::IStreamW &s, COPPAMgrSaveData &d) const {
    int version = k_saveGameVersion;
    s << version;
    SerializeV1(s, d);
}

// @0x5460fc
bool COPPAMgr::LoadDataFromStream(jet::stream::IStream &s, COPPAMgrSaveData &d) const {
    int version = 0;
    s >> version;
    return version == 1 && DeserializeV1(s, d);
}

// @0x546148
void COPPAMgr::SerializeV1(jet::stream::IStreamW &s, COPPAMgrSaveData &d) const {
    int sig = k_signatureV1;
    s << sig;
    int v = (int)d.flags;
    s << v;
    v = (int)d.prevFlags;
    s << v;
    s << d.lastEmailQuestionTime;
    s << d.birthYear;
    s << d.birthMonth;
    s << d.birthDay;
}

// @0x5461d4
bool COPPAMgr::DeserializeV1(jet::stream::IStream &s, COPPAMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    int v = 0;
    s >> v;
    d.flags = (unsigned)v;
    s >> v;
    d.prevFlags = (unsigned)v;
    s >> d.lastEmailQuestionTime;
    s >> d.birthYear;
    s >> d.birthMonth;
    s >> d.birthDay;
    return true;
}

// @0x5467a4: the country of the device; the first time the minimum age rule is set for it
void COPPAMgr::TryToEnableCOPPA() {
    m_country = GetDeviceCountry();
    if (HasFlag(0x40) || HasFlag(0x80)) return;
    m_data.flags = 0;
    SetFlag(GetCountryInfo() ? 0x40 : 0x80, true);
}

// @0x5466d8: the SIM card's country, else the phone's region, else "IT". This host has neither (nativeGetSimcardCountry
// and nativeGetPhoneRegion are Android calls), so it takes the binary's own fallback.
jet::String COPPAMgr::GetDeviceCountry() { return jet::String("IT"); }

// @0x5463e8
const COPPAMgr::CountryInfo *COPPAMgr::GetCountryInfo() const {
    const std::vector<CountryInfo> &db = CountryInfoDB();
    for (unsigned i = 0; i < k_countryInfoDBSize; i++)
        if (strcasecmp(m_country.c_str(), db[i].code.c_str()) == 0) return &db[i];
    return &db[k_fallbackCountry];
}

// @0x54627c: in this build every path (the country's rules and the user's age) ends in false
bool COPPAMgr::AreSocialFeaturesEnabled() const { return false; }

// @0x54645c: the OFT countries show the contact section to who has not met the minimum age
bool COPPAMgr::MustShowContactSection() const {
    const CountryInfo *c = GetCountryInfo();
    if (!c) return false;
    return c->oft ? !IsMinimumAgeMet() : false;
}

// @0x546b74: Italy, and Vietnam with it
bool COPPAMgr::IsItaliaCountry() const { return strcasecmp(m_country.c_str(), "IT") == 0 || strcasecmp(m_country.c_str(), "VN") == 0; }

// @0x546d94: "Euro" (else the euro sign) becomes the currency sign the fonts have
jet::String COPPAMgr::FixEURSymbol(const jet::String &text) {
    const char *euro = "\xe2\x82\xac", *currency = "\xc2\xa4";
    unsigned pos = text.find("Euro", 0);
    const char *what = "Euro";
    unsigned len = 4;
    if (pos == jet::String::npos) {
        pos = text.find(euro, 0);
        if (pos == jet::String::npos) return text;
        what = euro;
        len = 3;
    }
    std::string s(text.c_str());
    for (size_t p = (size_t)pos; p != std::string::npos; p = s.find(what, 0)) s.replace(p, len, currency);
    return jet::String(s.c_str());
}
