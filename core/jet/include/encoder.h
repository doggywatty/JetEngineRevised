// core/jet/include/encoder.h
#ifndef CORE_JET_INCLUDE_ENCODER_H
#define CORE_JET_INCLUDE_ENCODER_H

#include <vector>
#include "jtypes.h"

namespace jet {
namespace encoder {

u8 GetValueFromBase64(u8 c);
bool EncodeBase64(unsigned size, const u8* data, std::vector<u8>& out);
bool DecodeBase64(unsigned size, const u8* data, std::vector<u8>& out);

}
}

#endif
