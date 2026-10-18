// libs/vox1.1/include/adpcmstate.h
#ifndef LIBS_VOX1_1_INCLUDE_ADPCMSTATE_H
#define LIBS_VOX1_1_INCLUDE_ADPCMSTATE_H

// the predictor of one channel of an IMA ADPCM stream
class AdpcmState {  // sizeof 0x4
public:
    AdpcmState() : m_prevSample(0), m_prevIndex(0), _padding(0) {}
    AdpcmState(const AdpcmState &o) : m_prevSample(o.m_prevSample), m_prevIndex(o.m_prevIndex), _padding(0) {}
    ~AdpcmState() {}
    void SetPrevSample(short s) { m_prevSample = s; }
    short GetPrevSample() const { return m_prevSample; }
    void SetPrevIndex(unsigned char i) { m_prevIndex = i; }
    unsigned char GetPrevIndex() const { return m_prevIndex; }
    short m_prevSample;  // 0x0
    unsigned char m_prevIndex;  // 0x2
    unsigned char _padding;  // 0x3
};

#endif
