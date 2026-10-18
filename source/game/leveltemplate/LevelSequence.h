// source/game/leveltemplate/LevelSequence.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCE_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCE_H

#include <vector>
#include "core/pstring.h"
#include "game/leveltemplate/SpeedDef.h"

// One node of a parsed level description: a template number, an interval of them or a list of
// sub sequences, each repeated once, n times, forever or in random order
class LevelSequence {  // sizeof 0x44
public:
    enum SequenceValueType { VALUE_NONE, VALUE_NUMBER, VALUE_INTERVAL, VALUE_SEQUENCES };
    enum SequenceRepetitionType { REPEAT_ONCE, REPEAT_TIMES, REPEAT_FOREVER, REPEAT_RANDOM };
    LevelSequence();
    virtual ~LevelSequence();
    int GetNextSequenceNumber();
    void IncSequenceNumber();
    void Reset();
    const jet::String &GetCameraName();
    int GetCameraTransitionTime();
    SpeedDef *GetSpeed();
    const jet::String &GetLocation();
    void SetSequenceRepetitionType(SequenceRepetitionType t, int times) { m_sequenceRepetitionType = t; m_totalRepetitions = times; }
    void SetValue(int v);
    void SetInterval(int lo, int hi);
    void SetCamera(const jet::String &name, int transitionTime) { m_cameraName = name; m_cameraTransitionTime = transitionTime; }
    void SetSpeed(const SpeedDef &s);
    void SetLocation(const jet::String &l) { m_location = l; }
    void AddLevelSequence(LevelSequence *s);
    // back to the first of the sequence, the nested ones too
    void ResetSequence();
private:
    void OnSequenceFinished();
    int GetNextSequenceNumberInValue();
    int GetNextSequenceNumberInInterval();
    int GetNextSequenceNumberInSequences();
    void IncSequenceNumberInInterval();
    int GetRandomValueWithNoRepetition(int lo, int hi, int avoid);
    int m_index;                                    // 0x4
    int m_repetitions;                              // 0x8
    SequenceValueType m_sequenceValueType;          // 0xC
    SequenceRepetitionType m_sequenceRepetitionType;  // 0x10
    int m_totalRepetitions;                         // 0x14
    int m_value;                                    // 0x18
    int m_lowerBound;                               // 0x1C
    int m_upperBound;                               // 0x20
    std::vector<LevelSequence *> m_levelSequences;  // 0x24
    jet::String m_cameraName;                       // 0x34
    int m_cameraTransitionTime;                     // 0x38
    SpeedDef *m_speedValues;                        // 0x3C
    jet::String m_location;                         // 0x40
};

#endif
