// source/game/leveltemplate/LevelSequenceParser.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCEPARSER_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCEPARSER_H

#include <string>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "game/gameenums.h"
#include "game/leveltemplate/LevelSequence.h"

// Level descriptions: "[1,2-4,[5,6]/R,Camera{@cam,500}]/*" = lists, template numbers, intervals and the
// Camera{@entity,ms}, Speed{x}, Location{name} functions; each list ends "/n", "/*" (forever) or "/R" (random)
class LevelSequenceParser : public Singleton<LevelSequenceParser> {  // sizeof 0x18
public:
    typedef std::vector<std::string> Strings;
    LevelSequenceParser();
    virtual ~LevelSequenceParser() {}
    bool CheckLevelString(const jet::String &s);
    LevelSequence *BuildLevelDescription(const jet::String &s);
    bool IsValidFunction(const std::string &s);
    bool IsValidProbabilityFunction(const jet::String &s);
    bool IsValidSetTutorialFinishedFunction(const jet::String &s);
    bool IsValidExitFunction(const jet::String &s);
    bool IsValidTutorialFunction(const jet::String &s);
    void ProcessProbabilityFunction(const jet::String &s, int *weight, LevelSequence **seq);
    void ProcessTutorialFunction(const jet::String &s, LevelSequence **seq);
    void ProcessExitFunction(const jet::String &s, jet::String *label, ELocation *location);
    void ProcessConditionalEdge(const jet::String &s, jet::String *out);
private:
    static void CleanLevelDescription(std::string &s);
    bool IsValidList(const std::string &s);
    bool IsValidElemList(const std::string &s);
    bool IsValidElem(const std::string &s);
    bool IsValidInterval(const std::string &s);
    bool IsValidPosInt(const std::string &s);
    bool IsValidInt(const std::string &s);
    bool IsValidFloat(const std::string &s);
    bool IsValidLiteral(const std::string &s);
    bool IsValidFunctionParams(const std::string &s);
    bool IsValidFunctionParam(const std::string &s);
    bool IsValidEntity(const std::string &s);
    bool IsValidCamera(const Strings &f);
    bool IsValidSpeed(const Strings &f);
    bool IsValidLocation(const Strings &f);
    bool IsValidProbability(const Strings &f);
    static int GetParamNumber(const Strings &f) { return (int)f.size() - 1; }
    Strings ParseFunctionString(const std::string &s);
    // the index of the first comma outside brackets, the length when there is none, -1 on unbalanced brackets
    static unsigned GetFirstElemLimit(const std::string &s);
    LevelSequence *AddList(LevelSequence *parent, const std::string &s);
    void AddElemList(LevelSequence *parent, const std::string &s);
    void AddElem(LevelSequence *parent, const std::string &s);
    void AddInterval(LevelSequence *parent, const std::string &s);
    void ProcessElem(LevelSequence *seq, const std::string &s);
    void ProcessInterval(LevelSequence *seq, const std::string &s);
    void ProcessFunction(LevelSequence *seq, const std::string &s);
    void ProcessCamera(const Strings &f);
    void ProcessSpeed(const Strings &f);
    void ProcessLocation(const Strings &f);
    LevelSequence *NewSequence();
    jet::String m_currentCameraName;  // 0x4
    int m_currentCameraTransitionTime;  // 0x8
    SpeedDef m_currentSpeedDef;  // 0xC
    jet::String m_currentLocation;  // 0x14
};

#endif
