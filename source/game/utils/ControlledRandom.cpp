// source/game/utils/ControlledRandom.cpp
#include <algorithm>
#include <cstring>
#include "game/utils/ControlledRandom.h"
#include "system.h"

ControlledRandom::ControlledRandom() : m_startSeed(0), m_x(0), m_y(0), m_z(0), m_w(0) { SRand(); }

void ControlledRandom::SRand() { SRand((int)jet::System::GetTime()); }

void ControlledRandom::SRand(int seed) {
    m_startSeed = seed;
    m_x = seed;
    m_y = 0x32378fc7;
    m_z = 0xd55f8767;
    m_w = 0x104aa1ad;
    for (size_t i = 0; i < m_linked.size(); i++) m_linked[i]->SRand(Rand());
}

static int64_t Step(int64_t &x, int64_t &y, int64_t &z, int64_t &w) {
    int64_t t = (int64_t)((uint64_t)x ^ ((uint64_t)x << 11));
    x = y; y = z; z = w;
    w = w ^ (w >> 19) ^ t ^ (t >> 8);
    return w;
}

int ControlledRandom::Rand() {
    int64_t x = m_x, y = m_y, z = m_z, w = m_w;
    do Step(x, y, z, w); while ((w & 0x7fffffff) == 0x7fffffff);
    m_x = x; m_y = y; m_z = z; m_w = w;
    return (int)(w & 0x7fffffff);
}

int ControlledRandom::Rand(int a, int b) {
    if (b < a) std::swap(a, b);
    int r = Rand();
    int d = b - a;
    if (d == 0) return a;
    return a + (int)((int64_t)(r >> 2) % d);
}

int ControlledRandom::Rand2() { return (int)(Step(m_x, m_y, m_z, m_w) & 0x7fffffff); }

int ControlledRandom::Rand2(int a, int b) {
    unsigned r = (unsigned)Rand2();
    return a + (((b - a) * (int)((r & 0x3ffff) >> 2)) >> 16);
}

float ControlledRandom::Randf() { return (float)((Rand() & 0xffff) - 0x7fff) / 32768.f; }
float ControlledRandom::RandPosf() { return (float)(Rand() & 0xffff) / 65536.f; }

float ControlledRandom::Randf(float a, float b) {
    if (b < a) std::swap(a, b);
    return a + (b - a) * RandPosf();
}

float ControlledRandom::Randf2(unsigned &s) {
    s *= 0x41a7;
    unsigned bits = (s & 0x7fffff) | 0x40000000;
    float f;
    memcpy(&f, &bits, 4);
    return f - 3.f;
}

float ControlledRandom::RandPosf2(unsigned &s) { return Randf2(s) * 0.5f + 0.5f; }
float ControlledRandom::Randf2(unsigned &s, float a, float b) { return a + RandPosf2(s) * (b - a); }

void ControlledRandom::AddLinked(ControlledRandom *r) {
    if (r && std::find(m_linked.begin(), m_linked.end(), r) == m_linked.end()) m_linked.push_back(r);
}
