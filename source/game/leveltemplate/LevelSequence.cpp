// source/game/leveltemplate/LevelSequence.cpp
#include "game/leveltemplate/LevelSequence.h"
#include "game/GameLevel.h"
#include "game/gameenums.h"
#include "game/utils/ControlledRandom.h"

LevelSequence::LevelSequence()
    : m_index(0), m_repetitions(0), m_sequenceValueType(VALUE_NONE), m_sequenceRepetitionType(REPEAT_ONCE),
      m_totalRepetitions(0), m_value(-1), m_lowerBound(-1), m_upperBound(-1), m_cameraTransitionTime(0),
      m_speedValues(0), m_location(LocationIdToString(ELocation(ELocationDef::LABO_ROOM))) {}

LevelSequence::~LevelSequence() {
    for (size_t i = 0; i < m_levelSequences.size(); i++) delete m_levelSequences[i];
    delete m_speedValues;
}

void LevelSequence::SetValue(int v) {
    m_sequenceValueType = VALUE_NUMBER;
    SetSequenceRepetitionType(REPEAT_ONCE, 1);
    m_value = v;
}

void LevelSequence::SetInterval(int lo, int hi) {
    m_sequenceValueType = VALUE_INTERVAL;
    SetSequenceRepetitionType(REPEAT_ONCE, 1);
    m_lowerBound = lo;
    m_upperBound = hi;
}

void LevelSequence::AddLevelSequence(LevelSequence *s) {
    m_sequenceValueType = VALUE_SEQUENCES;
    m_levelSequences.push_back(s);
}

void LevelSequence::SetSpeed(const SpeedDef &s) {
    if (!m_speedValues) m_speedValues = new SpeedDef;
    m_speedValues->speed = s.speed;
    m_speedValues->isSet = true;
}

int LevelSequence::GetNextSequenceNumber() {
    switch (m_sequenceValueType) {
    case VALUE_INTERVAL: return GetNextSequenceNumberInInterval();
    case VALUE_SEQUENCES: return GetNextSequenceNumberInSequences();
    case VALUE_NUMBER: return GetNextSequenceNumberInValue();
    default: return -1;
    }
}

void LevelSequence::IncSequenceNumber() {
    if (m_sequenceValueType == VALUE_INTERVAL) {
        IncSequenceNumberInInterval();
    } else if (m_sequenceValueType == VALUE_SEQUENCES) {
        LevelSequence *c = m_levelSequences[m_index];
        c->IncSequenceNumber();
        if (c->GetNextSequenceNumber() == -1) OnSequenceFinished();
    } else if (m_sequenceValueType == VALUE_NUMBER) {
        m_repetitions++;
    }
}

int LevelSequence::GetNextSequenceNumberInValue() {
    switch (m_sequenceRepetitionType) {
    case REPEAT_ONCE: if (m_repetitions != 0) return -1; break;
    case REPEAT_TIMES: if (m_totalRepetitions <= m_repetitions) return -1; break;
    case REPEAT_FOREVER:
    case REPEAT_RANDOM: break;
    default: return -1;
    }
    return m_value;
}

// the interval and sequences cases share the repetition check
static bool RepetitionsDone(int type, int reps, int total) {
    switch (type) {
    case 0: case 3: return reps != 0;
    case 1: return total <= reps;
    case 2: return false;
    default: return true;
    }
}

int LevelSequence::GetNextSequenceNumberInInterval() {
    if (RepetitionsDone(m_sequenceRepetitionType, m_repetitions, m_totalRepetitions)) return -1;
    return m_lowerBound + m_index;
}

int LevelSequence::GetNextSequenceNumberInSequences() {
    if (RepetitionsDone(m_sequenceRepetitionType, m_repetitions, m_totalRepetitions)) return -1;
    LevelSequence *c = m_levelSequences[m_index];
    switch (c->m_sequenceValueType) {
    case VALUE_INTERVAL: return c->GetNextSequenceNumberInInterval();
    case VALUE_SEQUENCES: return c->GetNextSequenceNumberInSequences();
    case VALUE_NUMBER: return c->GetNextSequenceNumberInValue();
    default: return -1;
    }
}

void LevelSequence::IncSequenceNumberInInterval() {
    if (m_sequenceRepetitionType == REPEAT_RANDOM) {
        m_repetitions++;
    } else if (m_index + m_lowerBound == m_upperBound) {
        m_repetitions++;
        m_index = 0;
    } else {
        m_index++;
    }
}

void LevelSequence::OnSequenceFinished() {
    m_levelSequences[m_index]->ResetSequence();
    if (m_sequenceRepetitionType == REPEAT_RANDOM) {
        m_repetitions++;
    } else if (m_sequenceValueType == VALUE_SEQUENCES && m_index == (int)m_levelSequences.size() - 1) {
        m_repetitions++;
        m_index = 0;
    } else {
        m_index++;
    }
}

void LevelSequence::Reset() {
    m_repetitions = 0;
    m_index = 0;
    if (m_sequenceRepetitionType == REPEAT_RANDOM) {
        if (m_sequenceValueType == VALUE_INTERVAL)
            m_index = GetRandomValueWithNoRepetition(0, m_upperBound - m_lowerBound + 1, -1);
        else if (m_sequenceValueType == VALUE_SEQUENCES)
            m_index = GetRandomValueWithNoRepetition(0, (int)m_levelSequences.size(), -1);
    }
    if (m_sequenceValueType == VALUE_SEQUENCES)
        for (size_t i = 0; i < m_levelSequences.size(); i++) m_levelSequences[i]->Reset();
}

void LevelSequence::ResetSequence() {
    m_repetitions = 0;
    if (m_sequenceRepetitionType != REPEAT_RANDOM) {
        if (m_sequenceValueType == VALUE_SEQUENCES)
            for (size_t i = 0; i < m_levelSequences.size(); i++) m_levelSequences[i]->ResetSequence();
        m_index = 0;
        return;
    }
    if (m_sequenceValueType == VALUE_INTERVAL)
        m_index = GetRandomValueWithNoRepetition(0, m_upperBound - m_lowerBound + 1, m_index);
    else if (m_sequenceValueType == VALUE_SEQUENCES)
        m_index = GetRandomValueWithNoRepetition(0, (int)m_levelSequences.size(), m_index);
}

// the current leaf's camera, speed or location
const jet::String &LevelSequence::GetCameraName() {
    static const jet::String empty;
    LevelSequence *s = this;
    for (; s->m_sequenceValueType > VALUE_NONE; s = s->m_levelSequences[s->m_index]) {
        if (s->m_sequenceValueType < VALUE_SEQUENCES) return s->m_cameraName;
        if (s->m_sequenceValueType != VALUE_SEQUENCES) break;
    }
    return empty;
}

int LevelSequence::GetCameraTransitionTime() {
    LevelSequence *s = this;
    for (;;) {
        if (s->m_sequenceValueType < VALUE_NUMBER) return 0;
        if (s->m_sequenceValueType < VALUE_SEQUENCES) return s->m_cameraTransitionTime;
        s = s->m_levelSequences[s->m_index];
    }
}

SpeedDef *LevelSequence::GetSpeed() {
    LevelSequence *s = this;
    for (;;) {
        if (s->m_sequenceValueType < VALUE_NUMBER) return 0;
        if (s->m_sequenceValueType < VALUE_SEQUENCES) return s->m_speedValues;
        s = s->m_levelSequences[s->m_index];
    }
}

const jet::String &LevelSequence::GetLocation() {
    LevelSequence *s = this;
    while (s->m_sequenceValueType == VALUE_SEQUENCES) s = s->m_levelSequences[s->m_index];
    return s->m_location;
}

int LevelSequence::GetRandomValueWithNoRepetition(int lo, int hi, int avoid) {
    if (hi - lo > 1) {
        int r;
        do r = GameLevel::s_randomizer[0]->Rand(lo, hi); while (r == avoid);
        return r;
    }
    return lo;
}
