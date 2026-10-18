// source/game/leveltemplate/LevelSequenceParser.cpp
#include <cstdlib>
#include <cstring>
#include "game/leveltemplate/LevelSequenceParser.h"
#include "project.h"

typedef std::string Str;
static const Str::size_type npos = Str::npos;

LevelSequenceParser::LevelSequenceParser()
    : m_currentCameraTransitionTime(0), m_currentLocation(LocationIdToString(ELocation(ELocationDef::LABO_ROOM))) {
    m_currentSpeedDef.speed = -1.f;
}

void LevelSequenceParser::CleanLevelDescription(Str &s) {
    for (Str::size_type p; (p = s.find(' ')) != npos;) s.erase(p, 1);
}

bool LevelSequenceParser::CheckLevelString(const jet::String &js) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    return IsValidList(s);
}

bool LevelSequenceParser::IsValidList(const Str &s) {
    if (s.empty() || s[0] != '[') return false;
    Str::size_type p = s.rfind(']');
    if (p == npos) return false;
    char c = p + 2 <= s.size() ? s.c_str()[p + 2] : 0;
    if (p + 2 > s.size() || s[p + 1] != '/' || !(c == '*' || (c & 0xdf) == 'R' || (c >= '1' && c <= '9')) || p == 1) return false;
    return IsValidElemList(s.substr(1, p - 1));
}

bool LevelSequenceParser::IsValidElemList(const Str &s) {
    unsigned lim = GetFirstElemLimit(s);
    if (lim == (unsigned)-1) return false;
    Str e = s.substr(0, lim);
    if (!(IsValidElem(e) || IsValidInterval(e) || IsValidList(e) || IsValidFunction(e))) return false;
    if (lim == s.size()) return true;
    return IsValidElemList(s.substr(lim + 1));
}

bool LevelSequenceParser::IsValidFloat(const Str &s) { return s.find_first_not_of("-0123456789.,") == npos; }
bool LevelSequenceParser::IsValidInt(const Str &s) { return s.find_first_not_of("-0123456789") == npos; }
bool LevelSequenceParser::IsValidPosInt(const Str &s) { return s.find_first_not_of("0123456789") == npos; }
bool LevelSequenceParser::IsValidElem(const Str &s) { return s.find_first_not_of("0123456789") == npos; }

bool LevelSequenceParser::IsValidInterval(const Str &s) {
    Str::size_type p = s.find('-');
    if (p == 0 || p == npos) return false;
    Str a = s.substr(0, p), b = s.substr(p + 1);
    return IsValidElem(a) && IsValidElem(b) && atoi(a.c_str()) < atoi(b.c_str());
}

bool LevelSequenceParser::IsValidFunction(const Str &s) {
    Str::size_type a = s.find('{'), b = s.rfind('}');
    if (a == npos || b == npos || b < s.size() - 1) return false;
    return IsValidLiteral(s.substr(0, a)) && IsValidFunctionParams(s.substr(a + 1, b - a - 1));
}

bool LevelSequenceParser::IsValidLiteral(const Str &s) {
    unsigned char c = s.empty() ? 0 : (unsigned char)s[0];
    if (s.find_first_of("[]{},*") != npos) return false;
    return (unsigned char)((c & 0xdf) - 'A') < 26;
}

LevelSequenceParser::Strings LevelSequenceParser::ParseFunctionString(const Str &s) {
    Strings out;
    Str::size_type a = s.find('{'), b = s.rfind('}');
    out.push_back(s.substr(0, a));
    Str rest = s.substr(a + 1, b - a - 1);
    do {
        unsigned lim = GetFirstElemLimit(rest);
        out.push_back(rest.substr(0, lim));
        if (lim == rest.size()) break;
        rest = rest.substr(lim + 1);
    } while (!rest.empty());
    return out;
}

bool LevelSequenceParser::IsValidFunctionParams(const Str &s) {
    unsigned lim = GetFirstElemLimit(s);
    if (lim == (unsigned)-1) return false;
    bool ok = IsValidFunctionParam(s.substr(0, lim));
    if (ok && lim != s.size()) ok = IsValidFunctionParams(s.substr(lim + 1));
    return ok;
}

bool LevelSequenceParser::IsValidFunctionParam(const Str &s) {
    return IsValidElem(s) || IsValidFloat(s) || IsValidList(s) || IsValidLiteral(s) || IsValidEntity(s);
}

bool LevelSequenceParser::IsValidEntity(const Str &s) {
    if (s.empty() || s[0] != '@') return false;
    Str rest = s.substr(1);
    Str name = rest.substr(0, GetFirstElemLimit(rest));
    return clara::Project::GetInstance()->FindEntityByName(jet::String(name.c_str())) != 0;
}

bool LevelSequenceParser::IsValidCamera(const Strings &f) {
    return f[0] == "Camera" && GetParamNumber(f) == 2 && IsValidEntity(f[1]) && IsValidPosInt(f[2]);
}

bool LevelSequenceParser::IsValidProbability(const Strings &f) {
    return f[0] == "Probability" && GetParamNumber(f) == 2 && IsValidPosInt(f[1]) && IsValidList(f[2]);
}

bool LevelSequenceParser::IsValidSpeed(const Strings &f) { return f[0] == "Speed" && GetParamNumber(f) == 1 && IsValidFloat(f[1]); }
bool LevelSequenceParser::IsValidLocation(const Strings &f) { return f[0] == "Location" && GetParamNumber(f) == 1 && IsValidLiteral(f[1]); }

unsigned LevelSequenceParser::GetFirstElemLimit(const Str &s) {
    int depth = 0;
    unsigned i = 0;
    for (; i < s.size(); i++) {
        char c = s[i];
        if (c == ',' && depth == 0) return i;
        if ((c & 0xdf) == 0x5b) depth++;
        if ((c & 0xdf) == 0x5d) {
            if (depth == 0) return (unsigned)-1;
            depth--;
        }
    }
    return i;
}

LevelSequence *LevelSequenceParser::NewSequence() {
    LevelSequence *seq = new LevelSequence;
    seq->SetCamera(m_currentCameraName, m_currentCameraTransitionTime);
    seq->SetSpeed(m_currentSpeedDef);
    seq->SetLocation(m_currentLocation);
    return seq;
}

LevelSequence *LevelSequenceParser::BuildLevelDescription(const jet::String &js) {
    m_currentCameraName = jet::String();
    m_currentCameraTransitionTime = 0;
    m_currentSpeedDef.isSet = false;
    m_currentSpeedDef.speed = -1.f;
    m_currentLocation = LocationIdToString(ELocation(ELocationDef::LABO_ROOM));
    Str s = js.c_str();
    CleanLevelDescription(s);
    return AddList(0, s);
}

LevelSequence *LevelSequenceParser::AddList(LevelSequence *parent, const Str &s) {
    LevelSequence *seq = NewSequence();
    Str::size_type p = s.rfind(']');
    Str inner = s.substr(1, p - 1);
    if (IsValidElem(inner)) ProcessElem(seq, inner);
    else if (IsValidInterval(inner)) ProcessInterval(seq, inner);
    else if (IsValidFunction(inner)) ProcessFunction(seq, inner);
    else AddElemList(seq, inner);
    char c = s.c_str()[p + 2];
    if (c == '*') seq->SetSequenceRepetitionType(LevelSequence::REPEAT_FOREVER, 0);
    else if ((c & 0xdf) == 'R') seq->SetSequenceRepetitionType(LevelSequence::REPEAT_RANDOM, 0);
    else if (c >= '1' && c <= '9') seq->SetSequenceRepetitionType(LevelSequence::REPEAT_TIMES, atoi(s.c_str() + p + 2));
    if (parent) parent->AddLevelSequence(seq);
    return seq;
}

void LevelSequenceParser::AddElemList(LevelSequence *parent, const Str &s) {
    unsigned lim = GetFirstElemLimit(s);
    Str e = s.substr(0, lim);
    if (IsValidElem(e)) AddElem(parent, e);
    else if (IsValidInterval(e)) AddInterval(parent, e);
    else if (IsValidList(e)) AddList(parent, e);
    else if (IsValidFunction(e)) ProcessFunction(parent, e);
    if (lim != s.size()) AddElemList(parent, s.substr(lim + 1));
}

void LevelSequenceParser::AddElem(LevelSequence *parent, const Str &s) {
    LevelSequence *seq = NewSequence();
    ProcessElem(seq, s);
    parent->AddLevelSequence(seq);
}

void LevelSequenceParser::AddInterval(LevelSequence *parent, const Str &s) {
    LevelSequence *seq = NewSequence();
    ProcessInterval(seq, s);
    parent->AddLevelSequence(seq);
}

void LevelSequenceParser::ProcessElem(LevelSequence *seq, const Str &s) { seq->SetValue(atoi(s.c_str())); }

void LevelSequenceParser::ProcessInterval(LevelSequence *seq, const Str &s) {
    Str::size_type p = s.find('-');
    seq->SetInterval(atoi(s.substr(0, p).c_str()), atoi(s.substr(p + 1).c_str()));
}

void LevelSequenceParser::ProcessFunction(LevelSequence *, const Str &s) {
    Strings f = ParseFunctionString(s);
    if (IsValidCamera(f)) ProcessCamera(f);
    else if (IsValidLocation(f)) ProcessLocation(f);
    else if (IsValidSpeed(f)) ProcessSpeed(f);
}

void LevelSequenceParser::ProcessCamera(const Strings &f) {
    m_currentCameraName = jet::String(f[1].substr(f[1].find('@') + 1).c_str());
    m_currentCameraTransitionTime = atoi(f[2].c_str());
}

void LevelSequenceParser::ProcessSpeed(const Strings &f) {
    m_currentSpeedDef.isSet = true;
    m_currentSpeedDef.speed = (float)strtod(f[1].c_str(), 0);
}

void LevelSequenceParser::ProcessLocation(const Strings &f) { m_currentLocation = jet::String(f[1].c_str()); }

bool LevelSequenceParser::IsValidProbabilityFunction(const jet::String &js) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    return IsValidFunction(s) && IsValidProbability(ParseFunctionString(s));
}

bool LevelSequenceParser::IsValidSetTutorialFinishedFunction(const jet::String &js) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    if (!IsValidFunction(s)) return false;
    Strings f = ParseFunctionString(s);
    return f[0] == "SetTutorialFinished" && GetParamNumber(f) == 1 && (f[1] == "false" || f[1] == "true");
}

bool LevelSequenceParser::IsValidExitFunction(const jet::String &js) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    if (!IsValidFunction(s)) return false;
    Strings f = ParseFunctionString(s);
    return f[0] == "Exit" && GetParamNumber(f) == 2;
}

bool LevelSequenceParser::IsValidTutorialFunction(const jet::String &js) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    if (!IsValidFunction(s)) return false;
    Strings f = ParseFunctionString(s);
    return f[0] == "Tutorial" && GetParamNumber(f) == 1 && IsValidList(f[1]);
}

void LevelSequenceParser::ProcessProbabilityFunction(const jet::String &js, int *weight, LevelSequence **seq) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    Strings f = ParseFunctionString(s);
    *seq = AddList(0, f[2]);
    *weight = atoi(f[1].c_str());
}

void LevelSequenceParser::ProcessTutorialFunction(const jet::String &js, LevelSequence **seq) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    *seq = AddList(0, ParseFunctionString(s)[1]);
}

void LevelSequenceParser::ProcessExitFunction(const jet::String &js, jet::String *label, ELocation *location) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    Strings f = ParseFunctionString(s);
    *location = ELocation((ELocationDef::type)LocationStringToId(jet::String(f[1].c_str())));
    *label = jet::String(f[2].c_str());
}

void LevelSequenceParser::ProcessConditionalEdge(const jet::String &js, jet::String *out) {
    Str s = js.c_str();
    CleanLevelDescription(s);
    *out = jet::String(s.c_str());
}
