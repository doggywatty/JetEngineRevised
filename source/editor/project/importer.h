// source/editor/project/importer.h
#pragma once

#include "editor/project/project.h"

#include <string>

namespace jeteditor::project {

// Imports one source file into the project. The type is decided by the extension and then checked against the
// content, and the file is copied byte for byte into the folder for its type:
//   .pig                                  -> models/    pig: ReadPig, WritePig, output equal to the input
//   .tga                                  -> textures/  uncompressed 24/32-bit TGA: ReadTga, WriteTga, output equal
//   .tga or .pvr whose bytes start PVR\3  -> textures/  PVR3 (ETC1): ReadPvr3 checks the header and length
//   .xml                                  -> shaders/   XML that parses (pugixml) with a <technique> root element
// Any other extension, and any file that fails its check, is refused. The destination name is the source's file
// name; an existing project file with that name is refused, not replaced. On success a record (path, type,
// source, hash, size) is appended and assets/database.txt is saved. On failure the project is unchanged.
bool ImportAsset(Project &project, const std::string &sourcePath, std::string &error);

}  // namespace jeteditor::project
