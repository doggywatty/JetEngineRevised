// source/version.cpp
#include "version.h"

namespace despicableme {

// @0x42f914: a function static
const std::string &Version::GetGameVersion() {
    static const std::string version("0.0.3");
    return version;
}

// @0x3bcba4 (static init), read by GetGameCode @0x42fb08
const std::string Version::GAME_CODE("DMPH");
const std::string &Version::GetGameCode() { return GAME_CODE; }

}
