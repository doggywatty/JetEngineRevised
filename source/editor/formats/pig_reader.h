// source/editor/formats/pig_reader.h
//
// Reads a .pig file into a pig::Model. Every read is bounds-checked; a malformed or truncated file is reported
// through the error string and never read past its end. Trailing bytes after the optional source-path table are
// an error, so nothing is silently dropped.
#pragma once

#include "editor/formats/pig_model.h"

#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::pig {

bool ReadPig(const std::vector<uint8_t> &bytes, Model &out, std::string &error);
bool ReadPigFile(const std::string &path, Model &out, std::string &error);

}  // namespace jeteditor::pig
