// source/game/utils/ControlledRandom.h
#ifndef SOURCE_GAME_UTILS_CONTROLLEDRANDOM_H
#define SOURCE_GAME_UTILS_CONTROLLEDRANDOM_H

#include <cstdint>
#include <vector>

// Seedable xorshift generator; linked generators are reseeded from it
class ControlledRandom {  // sizeof 0x38
public:
    ControlledRandom();
    virtual ~ControlledRandom() {}
    int Rand(int a, int b);
    int Rand();
    int Rand2(int a, int b);
    int Rand2();
    float Randf(float a, float b);
    float Randf();
    float RandPosf();
    float Randf2(unsigned &s, float a, float b);
    float Randf2(unsigned &s);
    float RandPosf2(unsigned &s);
    int GetInitialSeed() const { return m_startSeed; }
    void SRand();
    void SRand(int seed);
    void AddLinked(ControlledRandom *r);
protected:
    std::vector<ControlledRandom *> m_linked;  // 0x4
    int m_startSeed;                           // 0x10
    int64_t m_x, m_y, m_z, m_w;                // 0x18
};

#endif
