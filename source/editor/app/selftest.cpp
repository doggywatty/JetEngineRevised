// source/editor/app/selftest.cpp
#include "editor/app/selftest.h"

#include "editor/app/model.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <set>
#include <vector>

namespace jeteditor::app {

namespace {

int g_checks = 0;
int g_failed = 0;

void Check(bool ok, const std::string &what) {
    ++g_checks;
    if (!ok) ++g_failed;
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
}

void PrintLog(const Log &log) {
    for (const LogLine &line : log.Lines()) std::printf("log  %s %s\n", line.error ? "error:" : "info: ", line.text.c_str());
}

// Prints the paths that are in one sorted list and not the other
void Differences(const std::vector<std::string> &expected, const std::vector<std::string> &actual) {
    for (const std::string &p : expected)
        if (!std::binary_search(actual.begin(), actual.end(), p)) std::printf("     missing: %s\n", p.c_str());
    for (const std::string &p : actual)
        if (!std::binary_search(expected.begin(), expected.end(), p)) std::printf("     extra:   %s\n", p.c_str());
}

}  // namespace

int RunSelfTest(const std::string &root) {
    g_checks = 0;
    g_failed = 0;
    std::printf("selftest %s\n", root.c_str());
    Editor ed;
    const bool opened = ed.OpenProject(root);
    PrintLog(ed.log);
    Check(opened, "project opens");
    if (!opened) {
        std::printf("FAIL: %d of %d checks failed\n", g_failed, g_checks);
        return 1;
    }

    std::vector<std::string> dbPaths;
    for (const project::AssetRecord &a : ed.Current().assets) dbPaths.push_back(a.path);
    std::sort(dbPaths.begin(), dbPaths.end());

    // The explorer tree is built from the database; its leaves must be exactly its records
    std::vector<std::string> treeOrder;
    CollectAssets(ed.Tree(), treeOrder);
    std::vector<std::string> treePaths = treeOrder;
    std::sort(treePaths.begin(), treePaths.end());
    const bool treeMatches = treePaths == dbPaths;
    Check(treeMatches, "explorer tree lists every database asset and no other (" + std::to_string(treePaths.size()) +
                           " of " + std::to_string(dbPaths.size()) + ")");
    if (!treeMatches) Differences(dbPaths, treePaths);

    std::vector<std::string> layout;
    for (const TreeNode &c : ed.Tree().children) layout.push_back(c.name);
    std::vector<std::string> folders(std::begin(project::kLayoutFolders), std::end(project::kLayoutFolders));
    std::sort(layout.begin(), layout.end());
    std::sort(folders.begin(), folders.end());
    Check(layout == folders, "tree top level is the six layout folders");

    // The files on disk under the layout folders must be the database's files
    std::vector<std::string> onDisk;
    std::string error;
    const bool scanned = ScanLayoutFiles(root, onDisk, error);
    if (!scanned) std::printf("     %s\n", error.c_str());
    const bool diskMatches = scanned && onDisk == dbPaths;
    Check(diskMatches, "no file on disk is missing from the database, and none is missing from disk");
    if (scanned && !diskMatches) Differences(dbPaths, onDisk);

    Check(ed.DiskProblems().empty(), "each asset file matches its database size and hash");
    for (const std::string &p : ed.DiskProblems()) std::printf("     %s\n", p.c_str());

    // Select the first asset in tree order and check the inspector's record for it
    Check(!treeOrder.empty(), "project has an asset to select");
    if (!treeOrder.empty()) {
        ed.Select(treeOrder.front());
        const project::AssetRecord *a = ed.SelectedAsset();
        Check(a != nullptr && a->path == treeOrder.front(), "selecting the first tree asset shows its record");
        if (a) {
            const bool hashOk = a->hash.size() == 71 && a->hash.compare(0, 7, "sha256:") == 0;
            Check(hashOk && !a->type.empty() && !a->source.empty() && a->size > 0,
                  "inspector fields: type, source, size and sha256 hash are set");
            std::printf("selected %s  type=%s size=%llu\n     hash=%s\n     source=%s\n", a->path.c_str(),
                        a->type.c_str(), static_cast<unsigned long long>(a->size), a->hash.c_str(), a->source.c_str());
        }
    }

    if (g_failed == 0) {
        std::printf("PASS: all %d checks\n", g_checks);
        return 0;
    }
    std::printf("FAIL: %d of %d checks failed\n", g_failed, g_checks);
    return 1;
}

}  // namespace jeteditor::app
