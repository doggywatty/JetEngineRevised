// source/editor/formats/pig_writer.h
//
// Writes a pig::Model in the .pig layout that ModelBase::LoadV100 reads. The writer emits exactly the stored
// values, so a model read by ReadPig writes back to the same bytes. A model that cannot be expressed in the
// format (a string over 65535 bytes, a count over 65535, a skin block on an unskinned mesh or missing on a
// skinned one) is refused with an error.
#pragma once

#include "editor/formats/pig_model.h"

#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::pig {

bool WritePig(const Model &model, std::vector<uint8_t> &out, std::string &error);
bool WritePigFile(const std::string &path, const Model &model, std::string &error);

}  // namespace jeteditor::pig
