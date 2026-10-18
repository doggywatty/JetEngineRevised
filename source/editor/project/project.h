// source/editor/project/project.h
//
// A JetEditor project: a folder holding project.jetproj (the manifest), the asset database assets/database.txt, and
// the folders assets/, levels/, models/, textures/, shaders/ and sounds/. The formats are in re/JETEDITOR_FORMATS.md.
// Every file is read with a full parse; a file that does not parse is an error and is never partly used.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::project {

constexpr int kFormatVersion = 1;
constexpr const char *kManifestFile = "project.jetproj";
constexpr const char *kDatabaseFile = "assets/database.txt";
constexpr const char *kDatabaseHeader = "jetdb=1";
constexpr const char *kLayoutFolders[] = {"assets", "levels", "models", "textures", "shaders", "sounds"};

struct Manifest {
    int format = kFormatVersion;
    std::string name;
};

// project.jetproj: lines "format=1" then "name=<name>", each ended by LF
std::string FormatManifest(const Manifest &manifest);
bool ParseManifest(const std::string &text, Manifest &out, std::string &error);

struct AssetRecord {
    std::string path;    // project-relative with '/', e.g. models/cube.pig
    std::string type;    // pig, tga, pvr3 or shader
    std::string source;  // the path the file was imported from, as given to the importer
    std::string hash;    // sha256:<64 lower-case hex digits> of the file at path
    uint64_t size = 0;   // bytes of the file at path
};

// assets/database.txt: the line "jetdb=1", then one line per asset with five tab-separated fields: path, type, source,
// hash, size. Paths are unique.
std::string FormatDatabase(const std::vector<AssetRecord> &assets);
bool ParseDatabase(const std::string &text, std::vector<AssetRecord> &out, std::string &error);

struct Project {
    std::string root;
    Manifest manifest;
    std::vector<AssetRecord> assets;
};

// Creates the folder layout, the manifest and an empty database. Refuses a folder that already holds a manifest.
bool CreateProject(const std::string &root, const std::string &name, Project &out, std::string &error);

// Reads the manifest and the database. Every layout folder must exist.
bool OpenProject(const std::string &root, Project &out, std::string &error);

// Writes assets/database.txt from project.assets
bool SaveDatabase(const Project &project, std::string &error);

const AssetRecord *FindAsset(const Project &project, const std::string &path);

// Re-reads every asset file and compares its size and hash with the database. Each problem is added to problems.
bool VerifyAssets(const Project &project, std::vector<std::string> &problems);

// A project path is relative, uses '/', has no empty, '.' or '..' part, contains no control characters, and starts
// with one of the layout folders.
bool IsSafeProjectPath(const std::string &path);

// File helpers: whole-file read and write, binary. Writing creates missing parent folders.
bool ReadFile(const std::string &path, std::vector<uint8_t> &out, std::string &error);
bool WriteFile(const std::string &path, const std::vector<uint8_t> &bytes, std::string &error);

}  // namespace jeteditor::project
