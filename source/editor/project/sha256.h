// source/editor/project/sha256.h
// SHA-256 (FIPS 180-4) over a byte buffer. Used for the content hashes in the project asset database.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace jeteditor::project {

// Lower-case hex digest, 64 characters
std::string Sha256Hex(const uint8_t *data, size_t size);

// "sha256:" followed by the hex digest, the form stored in the asset database
std::string ContentHash(const uint8_t *data, size_t size);

}  // namespace jeteditor::project
