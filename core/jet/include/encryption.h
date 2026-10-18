// core/jet/include/encryption.h
#ifndef CORE_JET_INCLUDE_ENCRYPTION_H
#define CORE_JET_INCLUDE_ENCRYPTION_H

#include <vector>
#include "core/pstring.h"

namespace jet {
namespace encryption {

// XTEA, 32 rounds; the 16-byte key is the password XOR-folded.
void Encipher(unsigned rounds, u32* v, const u32* key);
void Decipher(unsigned rounds, u32* v, const u32* key);
void BuildKey(u8* key, const String& password);
void EncipherXTEA(const u8* data, unsigned size, const String& password, std::vector<u32>& out);
void DecipherXTEA(u32* data, unsigned count, const String& password);

}
}

#endif
