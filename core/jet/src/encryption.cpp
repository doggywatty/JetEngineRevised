// core/jet/src/encryption.cpp
#include "encryption.h"
#include <cstring>

namespace jet {
namespace encryption {

static const u32 kDelta = 0x9e3779b9;

void Encipher(unsigned rounds, u32* v, const u32* key)
{
    u32 v0 = v[0], v1 = v[1], sum = 0;
    for (unsigned i = 0; i < rounds; ++i) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += kDelta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
    }
    v[0] = v0;
    v[1] = v1;
}

void Decipher(unsigned rounds, u32* v, const u32* key)
{
    u32 v0 = v[0], v1 = v[1], sum = kDelta * rounds;
    for (unsigned i = 0; i < rounds; ++i) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
        sum -= kDelta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
    }
    v[0] = v0;
    v[1] = v1;
}

void BuildKey(u8* key, const String& password)
{
    memset(key, 0, 16);
    for (unsigned i = 0; i < password.size(); ++i)
        key[i & 15] ^= u8(password.c_str()[i]);
}

void EncipherXTEA(const u8* data, unsigned size, const String& password, std::vector<u32>& out)
{
    unsigned padded = (size + 8) & ~7u;
    out.assign(padded / 4, 0);
    memcpy(out.data(), data, size);
    u32 key[4];
    BuildKey(reinterpret_cast<u8*>(key), password);
    for (unsigned i = 0; i < out.size(); i += 2)
        Encipher(32, &out[i], key);
}

void DecipherXTEA(u32* data, unsigned count, const String& password)
{
    u32 key[4];
    BuildKey(reinterpret_cast<u8*>(key), password);
    for (unsigned i = 0; i + 1 < count; i += 2)
        Decipher(32, data + i, key);
}

}
}
