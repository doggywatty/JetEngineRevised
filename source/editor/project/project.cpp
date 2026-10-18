// source/editor/project/project.cpp
#include "editor/project/project.h"

#include "editor/project/sha256.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

namespace jeteditor::project {
namespace fs = std::filesystem;
namespace {

bool HasControlCharacter(const std::string &s) {
    for (unsigned char c : s)
        if (c < 0x20 || c == 0x7f) return true;
    return false;
}

bool IsValidName(const std::string &name) {
    return !name.empty() && name.size() <= 200 && !HasControlCharacter(name);
}

// Splits text into lines. The text must end with LF; the LF is not part of a line.
bool SplitLines(const std::string &text, std::vector<std::string> &lines, std::string &error) {
    if (text.empty() || text.back() != '\n') {
        error = "file is empty or does not end with a newline";
        return false;
    }
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        lines.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return true;
}

bool ParseSize(const std::string &s, uint64_t &out) {
    if (s.empty() || s.size() > 19) return false;
    uint64_t v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        v = v * 10 + static_cast<uint64_t>(c - '0');
    }
    out = v;
    return true;
}

bool IsValidHash(const std::string &h) {
    if (h.size() != 7 + 64 || h.compare(0, 7, "sha256:") != 0) return false;
    for (size_t i = 7; i < h.size(); i++) {
        char c = h[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    }
    return true;
}

bool IsKnownType(const std::string &t) { return t == "pig" || t == "tga" || t == "pvr3" || t == "shader"; }

std::string ManifestPath(const std::string &root) { return (fs::path(root) / kManifestFile).string(); }
std::string DatabasePath(const std::string &root) { return (fs::path(root) / kDatabaseFile).string(); }

}  // namespace

std::string FormatManifest(const Manifest &manifest) {
    return "format=" + std::to_string(manifest.format) + "\nname=" + manifest.name + "\n";
}

bool ParseManifest(const std::string &text, Manifest &out, std::string &error) {
    out = Manifest();
    std::vector<std::string> lines;
    if (!SplitLines(text, lines, error)) return false;
    bool haveFormat = false, haveName = false;
    for (const std::string &line : lines) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            error = "manifest line without '=': " + line;
            return false;
        }
        std::string key = line.substr(0, eq), value = line.substr(eq + 1);
        if (key == "format") {
            if (haveFormat) {
                error = "manifest repeats format";
                return false;
            }
            if (value != std::to_string(kFormatVersion)) {
                error = "unsupported manifest format '" + value + "' (this editor reads format 1)";
                return false;
            }
            haveFormat = true;
        } else if (key == "name") {
            if (haveName) {
                error = "manifest repeats name";
                return false;
            }
            if (!IsValidName(value)) {
                error = "manifest name is empty, too long or has control characters";
                return false;
            }
            out.name = value;
            haveName = true;
        } else {
            error = "unknown manifest key '" + key + "'";
            return false;
        }
    }
    if (!haveFormat || !haveName) {
        error = "manifest needs both format and name";
        return false;
    }
    return true;
}

std::string FormatDatabase(const std::vector<AssetRecord> &assets) {
    std::string out = std::string(kDatabaseHeader) + "\n";
    for (const AssetRecord &a : assets)
        out += a.path + "\t" + a.type + "\t" + a.source + "\t" + a.hash + "\t" + std::to_string(a.size) + "\n";
    return out;
}

bool ParseDatabase(const std::string &text, std::vector<AssetRecord> &out, std::string &error) {
    out.clear();
    std::vector<std::string> lines;
    if (!SplitLines(text, lines, error)) return false;
    if (lines.empty() || lines[0] != kDatabaseHeader) {
        error = std::string("database does not start with ") + kDatabaseHeader;
        return false;
    }
    for (size_t i = 1; i < lines.size(); i++) {
        const std::string &line = lines[i];
        std::vector<std::string> f;
        size_t start = 0;
        while (true) {
            size_t tab = line.find('\t', start);
            if (tab == std::string::npos) {
                f.push_back(line.substr(start));
                break;
            }
            f.push_back(line.substr(start, tab - start));
            start = tab + 1;
        }
        if (f.size() != 5) {
            error = "database line " + std::to_string(i + 1) + " has " + std::to_string(f.size()) +
                    " fields (want 5)";
            return false;
        }
        AssetRecord a;
        a.path = f[0];
        a.type = f[1];
        a.source = f[2];
        a.hash = f[3];
        if (!IsSafeProjectPath(a.path)) {
            error = "database line " + std::to_string(i + 1) + ": unsafe path '" + a.path + "'";
            return false;
        }
        if (!IsKnownType(a.type)) {
            error = "database line " + std::to_string(i + 1) + ": unknown type '" + a.type + "'";
            return false;
        }
        if (a.source.empty() || HasControlCharacter(a.source)) {
            error = "database line " + std::to_string(i + 1) + ": bad source path";
            return false;
        }
        if (!IsValidHash(a.hash)) {
            error = "database line " + std::to_string(i + 1) + ": bad hash '" + a.hash + "'";
            return false;
        }
        if (!ParseSize(f[4], a.size)) {
            error = "database line " + std::to_string(i + 1) + ": bad size '" + f[4] + "'";
            return false;
        }
        for (const AssetRecord &prev : out) {
            if (prev.path == a.path) {
                error = "database repeats path " + a.path;
                return false;
            }
        }
        out.push_back(std::move(a));
    }
    return true;
}

bool CreateProject(const std::string &root, const std::string &name, Project &out, std::string &error) {
    if (!IsValidName(name)) {
        error = "project name is empty, too long or has control characters";
        return false;
    }
    std::error_code ec;
    if (fs::exists(ManifestPath(root), ec)) {
        error = root + " already holds a project";
        return false;
    }
    fs::create_directories(root, ec);
    if (ec) {
        error = "cannot create " + root + ": " + ec.message();
        return false;
    }
    for (const char *folder : kLayoutFolders) {
        fs::create_directories(fs::path(root) / folder, ec);
        if (ec) {
            error = "cannot create folder " + std::string(folder) + ": " + ec.message();
            return false;
        }
    }
    out = Project();
    out.root = root;
    out.manifest.name = name;
    std::vector<uint8_t> manifestBytes;
    const std::string text = FormatManifest(out.manifest);
    manifestBytes.assign(text.begin(), text.end());
    if (!WriteFile(ManifestPath(root), manifestBytes, error)) return false;
    return SaveDatabase(out, error);
}

bool OpenProject(const std::string &root, Project &out, std::string &error) {
    out = Project();
    out.root = root;
    std::vector<uint8_t> bytes;
    if (!ReadFile(ManifestPath(root), bytes, error)) return false;
    if (!ParseManifest(std::string(bytes.begin(), bytes.end()), out.manifest, error)) {
        error = std::string(kManifestFile) + ": " + error;
        return false;
    }
    if (!ReadFile(DatabasePath(root), bytes, error)) return false;
    if (!ParseDatabase(std::string(bytes.begin(), bytes.end()), out.assets, error)) {
        error = std::string(kDatabaseFile) + ": " + error;
        return false;
    }
    std::error_code ec;
    for (const char *folder : kLayoutFolders) {
        if (!fs::is_directory(fs::path(root) / folder, ec)) {
            error = "project folder missing: " + std::string(folder);
            return false;
        }
    }
    return true;
}

bool SaveDatabase(const Project &project, std::string &error) {
    const std::string text = FormatDatabase(project.assets);
    std::vector<uint8_t> bytes(text.begin(), text.end());
    return WriteFile(DatabasePath(project.root), bytes, error);
}

const AssetRecord *FindAsset(const Project &project, const std::string &path) {
    for (const AssetRecord &a : project.assets)
        if (a.path == path) return &a;
    return nullptr;
}

bool VerifyAssets(const Project &project, std::vector<std::string> &problems) {
    problems.clear();
    for (const AssetRecord &a : project.assets) {
        std::vector<uint8_t> bytes;
        std::string error;
        const std::string full = (fs::path(project.root) / a.path).string();
        if (!ReadFile(full, bytes, error)) {
            problems.push_back("missing: " + a.path);
            continue;
        }
        if (bytes.size() != a.size) {
            problems.push_back("size differs: " + a.path + " (database " + std::to_string(a.size) + ", file " +
                               std::to_string(bytes.size()) + ")");
            continue;
        }
        if (ContentHash(bytes.data(), bytes.size()) != a.hash) problems.push_back("hash differs: " + a.path);
    }
    return problems.empty();
}

bool IsSafeProjectPath(const std::string &path) {
    if (path.empty() || path.size() > 255 || HasControlCharacter(path)) return false;
    if (path.find('\\') != std::string::npos || path.find(':') != std::string::npos) return false;
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        size_t slash = path.find('/', start);
        std::string part = path.substr(start, slash == std::string::npos ? std::string::npos : slash - start);
        if (part.empty() || part == "." || part == "..") return false;
        parts.push_back(part);
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    if (parts.size() < 2) return false;
    for (const char *folder : kLayoutFolders)
        if (parts[0] == folder) return true;
    return false;
}

bool ReadFile(const std::string &path, std::vector<uint8_t> &out, std::string &error) {
    out.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        error = "cannot open " + path;
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    if (in.bad()) {
        error = "read failed for " + path;
        out.clear();
        return false;
    }
    return true;
}

bool WriteFile(const std::string &path, const std::vector<uint8_t> &bytes, std::string &error) {
    std::error_code ec;
    fs::path parent = fs::path(path).parent_path();
    if (!parent.empty()) fs::create_directories(parent, ec);
    if (ec) {
        error = "cannot create folder for " + path + ": " + ec.message();
        return false;
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "cannot create " + path;
        return false;
    }
    out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    out.close();
    if (!out) {
        error = "write failed for " + path;
        return false;
    }
    return true;
}

}  // namespace jeteditor::project
