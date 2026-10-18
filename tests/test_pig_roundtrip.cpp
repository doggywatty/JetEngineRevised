// Reads every .pig in a directory with the editor's pig reader, writes each model back, and requires the written bytes
// to equal the file byte for byte. A file that differs is reported with the first differing offset and fails the test.
// Also checks that the reader refuses a truncated file and a file with trailing bytes.
//   test_pig_roundtrip [models dir]     (default: data/models)
#include "editor/formats/pig_reader.h"
#include "editor/formats/pig_writer.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace jeteditor::pig;

namespace {

bool ReadBytes(const std::string &path, std::vector<uint8_t> &bytes) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    return true;
}

}  // namespace

int main(int argc, char **argv) {
    namespace fs = std::filesystem;
    const std::string dir = argc > 1 ? argv[1] : "data/models";

    std::vector<std::string> files;
    std::error_code ec;
    for (const fs::directory_entry &e : fs::directory_iterator(dir, ec))
        if (e.is_regular_file() && e.path().extension() == ".pig") files.push_back(e.path().string());
    if (ec || files.empty()) {
        std::fprintf(stderr, "no .pig files in %s\n", dir.c_str());
        return 1;
    }
    std::sort(files.begin(), files.end());

    size_t identical = 0;
    int failed = 0;
    for (const std::string &path : files) {
        std::vector<uint8_t> bytes;
        if (!ReadBytes(path, bytes)) {
            std::printf("FAIL read %s: cannot open\n", path.c_str());
            failed++;
            continue;
        }
        Model model;
        std::string error;
        if (!ReadPig(bytes, model, error)) {
            std::printf("FAIL parse %s: %s\n", path.c_str(), error.c_str());
            failed++;
            continue;
        }
        std::vector<uint8_t> written;
        if (!WritePig(model, written, error)) {
            std::printf("FAIL write %s: %s\n", path.c_str(), error.c_str());
            failed++;
            continue;
        }
        if (written == bytes) {
            identical++;
            continue;
        }
        size_t n = std::min(bytes.size(), written.size()), off = 0;
        while (off < n && bytes[off] == written[off]) off++;
        std::printf("DIFF %s: first difference at offset %zu (file %zu bytes, written %zu bytes)\n", path.c_str(), off,
                    bytes.size(), written.size());
        failed++;
    }

    // Negative checks on the first file: the reader must refuse damaged input rather than accept it.
    std::vector<uint8_t> first;
    if (ReadBytes(files[0], first)) {
        Model m;
        std::string error;
        std::vector<uint8_t> cut(first.begin(), first.begin() + first.size() / 2);
        if (ReadPig(cut, m, error)) {
            std::printf("FAIL truncated %s was accepted\n", files[0].c_str());
            failed++;
        }
        std::vector<uint8_t> padded = first;
        padded.push_back(0);
        if (ReadPig(padded, m, error)) {
            std::printf("FAIL %s with a trailing byte was accepted\n", files[0].c_str());
            failed++;
        }
    }

    std::printf("%zu of %zu pig files byte-identical after read -> write\n", identical, files.size());
    return failed ? 1 : 0;
}
