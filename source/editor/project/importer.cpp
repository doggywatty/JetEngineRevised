// source/editor/project/importer.cpp
#include "editor/project/importer.h"

#include "editor/formats/pig_reader.h"
#include "editor/formats/pig_writer.h"
#include "editor/formats/pvr3.h"
#include "editor/formats/tga.h"
#include "editor/project/sha256.h"

#include "pugixml.hpp"

#include <cctype>
#include <filesystem>
#include <system_error>

namespace jeteditor::project {
namespace fs = std::filesystem;
namespace {

std::string Lower(std::string s) {
    for (char &c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// The written form must equal the file, or the importer would store something other than what it read
bool RoundTripsTo(const std::vector<uint8_t> &original, const std::vector<uint8_t> &written, const char *what,
                  std::string &error) {
    if (written == original) return true;
    size_t n = original.size() < written.size() ? original.size() : written.size(), off = 0;
    while (off < n && original[off] == written[off]) off++;
    error = std::string(what) + " does not round-trip: first difference at byte " + std::to_string(off) + " (file " +
            std::to_string(original.size()) + " bytes, written " + std::to_string(written.size()) + " bytes)";
    return false;
}

bool CheckPig(const std::vector<uint8_t> &bytes, std::string &error) {
    pig::Model model;
    if (!pig::ReadPig(bytes, model, error)) {
        error = "pig does not read: " + error;
        return false;
    }
    std::vector<uint8_t> written;
    if (!pig::WritePig(model, written, error)) {
        error = "pig does not write: " + error;
        return false;
    }
    return RoundTripsTo(bytes, written, "pig", error);
}

bool CheckTga(const std::vector<uint8_t> &bytes, std::string &error) {
    tga::Image image;
    if (!tga::ReadTga(bytes, image, error)) {
        error = "TGA does not read: " + error;
        return false;
    }
    std::vector<uint8_t> written;
    if (!tga::WriteTga(image, written, error)) {
        error = "TGA does not write: " + error;
        return false;
    }
    return RoundTripsTo(bytes, written, "TGA", error);
}

bool CheckPvr3(const std::vector<uint8_t> &bytes, std::string &error) {
    pvr3::Info info;
    if (!pvr3::ReadPvr3(bytes, info, error)) {
        error = "PVR3 does not read: " + error;
        return false;
    }
    return true;
}

bool CheckShader(const std::vector<uint8_t> &bytes, std::string &error) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(bytes.data(), bytes.size());
    if (!result) {
        error = std::string("shader XML does not parse: ") + result.description() + " at offset " +
                std::to_string(result.offset);
        return false;
    }
    const std::string root = doc.document_element().name();
    if (root != "technique") {
        error = "shader XML root element is <" + root + ">, expected <technique>";
        return false;
    }
    return true;
}

}  // namespace

bool ImportAsset(Project &project, const std::string &sourcePath, std::string &error) {
    if (sourcePath.empty()) {
        error = "empty source path";
        return false;
    }
    for (unsigned char c : sourcePath)
        if (c < 0x20 || c == 0x7f) {
            error = "source path has control characters: " + sourcePath;
            return false;
        }

    const fs::path src(sourcePath);
    const std::string base = src.filename().string();
    const std::string ext = Lower(src.extension().string());
    if (base.empty() || base == "." || base == "..") {
        error = "source has no file name: " + sourcePath;
        return false;
    }

    std::vector<uint8_t> bytes;
    if (!ReadFile(sourcePath, bytes, error)) return false;

    std::string type, folder;
    if (ext == ".pig") {
        type = "pig";
        folder = "models";
        if (!CheckPig(bytes, error)) return false;
    } else if (ext == ".tga" && pvr3::IsPvr3(bytes)) {
        // The engine's Android textures are PVR3 under a .tga name; the content decides
        type = "pvr3";
        folder = "textures";
        if (!CheckPvr3(bytes, error)) return false;
    } else if (ext == ".tga") {
        type = "tga";
        folder = "textures";
        if (!CheckTga(bytes, error)) return false;
    } else if (ext == ".pvr") {
        if (!pvr3::IsPvr3(bytes)) {
            error = ".pvr file does not start with PVR\\3: " + sourcePath;
            return false;
        }
        type = "pvr3";
        folder = "textures";
        if (!CheckPvr3(bytes, error)) return false;
    } else if (ext == ".xml") {
        type = "shader";
        folder = "shaders";
        if (!CheckShader(bytes, error)) return false;
    } else {
        error = "unsupported asset type '" + ext + "' for " + sourcePath;
        return false;
    }

    const std::string path = folder + "/" + base;
    if (!IsSafeProjectPath(path)) {
        error = "file name cannot be used in a project: " + base;
        return false;
    }
    if (FindAsset(project, path)) {
        error = path + " is already in the project";
        return false;
    }
    const std::string full = (fs::path(project.root) / path).string();
    std::error_code ec;
    if (fs::exists(full, ec)) {
        error = full + " already exists";
        return false;
    }

    AssetRecord record;
    record.path = path;
    record.type = type;
    record.source = sourcePath;
    record.hash = ContentHash(bytes.data(), bytes.size());
    record.size = bytes.size();

    if (!WriteFile(full, bytes, error)) return false;
    project.assets.push_back(record);
    if (!SaveDatabase(project, error)) {
        project.assets.pop_back();
        fs::remove(full, ec);
        return false;
    }
    return true;
}

}  // namespace jeteditor::project
