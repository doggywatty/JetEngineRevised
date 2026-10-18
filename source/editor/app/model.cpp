// source/editor/app/model.cpp
#include "editor/app/model.h"

#include "editor/project/importer.h"

#include <algorithm>
#include <filesystem>
#include <system_error>

namespace jeteditor::app {

namespace fs = std::filesystem;
using project::AssetRecord;
using project::Project;

namespace {

std::vector<std::string> SplitPath(const std::string &path) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= path.size()) {
        size_t slash = path.find('/', start);
        if (slash == std::string::npos) slash = path.size();
        parts.push_back(path.substr(start, slash - start));
        start = slash + 1;
    }
    return parts;
}

// Folders first, then by name
void SortTree(TreeNode &node) {
    std::sort(node.children.begin(), node.children.end(), [](const TreeNode &a, const TreeNode &b) {
        if (a.folder != b.folder) return a.folder;
        return a.name < b.name;
    });
    for (TreeNode &c : node.children) SortTree(c);
}

void Visit(const TreeNode &node, int depth, const std::set<std::string> &expanded, std::vector<Row> &rows) {
    for (const TreeNode &child : node.children) {
        rows.push_back(Row{&child, depth});
        if (child.folder && expanded.count(child.path)) Visit(child, depth + 1, expanded, rows);
    }
}

}  // namespace

bool BuildTree(const Project &project, TreeNode &out, std::string &error) {
    TreeNode root;
    root.name = project.manifest.name;
    root.folder = true;
    for (const char *folder : project::kLayoutFolders) {
        TreeNode node;
        node.name = folder;
        node.path = folder;
        node.folder = true;
        root.children.push_back(node);
    }
    for (const AssetRecord &asset : project.assets) {
        const std::vector<std::string> parts = SplitPath(asset.path);
        if (parts.size() < 2) {
            error = "asset path has one part: " + asset.path;
            return false;
        }
        TreeNode *cur = &root;
        std::string prefix;
        for (size_t i = 0; i < parts.size(); ++i) {
            const bool last = i + 1 == parts.size();
            prefix += (i == 0 ? std::string() : std::string("/")) + parts[i];
            TreeNode *found = nullptr;
            for (TreeNode &c : cur->children)
                if (c.name == parts[i]) {
                    found = &c;
                    break;
                }
            if (!found) {
                cur->children.push_back(TreeNode{});
                found = &cur->children.back();
                found->name = parts[i];
                found->path = prefix;
                found->folder = !last;
            } else if (last || found->folder != !last) {
                error = "path " + asset.path + " is both a folder and an asset, or repeats one";
                return false;
            }
            cur = found;
        }
    }
    SortTree(root);
    out = root;
    return true;
}

void CollectAssets(const TreeNode &node, std::vector<std::string> &out) {
    if (!node.folder && !node.path.empty()) out.push_back(node.path);
    for (const TreeNode &c : node.children) CollectAssets(c, out);
}

void VisibleRows(const TreeNode &root, const std::set<std::string> &expanded, std::vector<Row> &rows) {
    rows.clear();
    Visit(root, 0, expanded, rows);
}

bool ScanLayoutFiles(const std::string &root, std::vector<std::string> &out, std::string &error) {
    out.clear();
    std::error_code ec;
    for (const char *folder : project::kLayoutFolders) {
        const fs::path base = fs::path(root) / folder;
        if (!fs::is_directory(base, ec)) {
            error = std::string("project folder missing: ") + folder;
            return false;
        }
        for (fs::recursive_directory_iterator it(base, ec), end; !ec && it != end; it.increment(ec)) {
            if (!it->is_regular_file(ec)) continue;
            const std::string rel = fs::relative(it->path(), root, ec).generic_string();
            if (rel == project::kDatabaseFile) continue;
            out.push_back(rel);
        }
        if (ec) {
            error = "cannot list " + base.string() + ": " + ec.message();
            return false;
        }
    }
    std::sort(out.begin(), out.end());
    return true;
}

bool Editor::NewProject(const std::string &root, const std::string &name) {
    Project p;
    std::string error;
    if (!project::CreateProject(root, name, p, error)) {
        log.Error("New project failed: " + error);
        return false;
    }
    log.Info("Created project \"" + name + "\" in " + root);
    return Adopt(std::move(p), true);
}

bool Editor::OpenProject(const std::string &root) {
    Project p;
    std::string error;
    if (!project::OpenProject(root, p, error)) {
        log.Error("Open project failed: " + error);
        return false;
    }
    return Adopt(std::move(p), true);
}

bool Editor::ImportAsset(const std::string &source) {
    if (!open_) {
        log.Error("Import asset failed: no project is open. Use File > New project or File > Open project first.");
        return false;
    }
    std::string error;
    if (!project::ImportAsset(project_, source, error)) {
        log.Error("Import asset failed: " + error);
        return false;
    }
    const AssetRecord &rec = project_.assets.back();
    log.Info("Imported " + rec.path + " (" + rec.type + ", " + std::to_string(rec.size) + " bytes) from " + source);
    selected_ = rec.path;
    return Adopt(project_, false);
}

const AssetRecord *Editor::SelectedAsset() const {
    if (!open_ || selected_.empty()) return nullptr;
    return project::FindAsset(project_, selected_);
}

bool Editor::Adopt(Project db, bool logDiskState) {
    TreeNode tree;
    std::string error;
    if (!BuildTree(db, tree, error)) {
        log.Error("Project cannot be shown: " + error);
        return false;
    }
    std::vector<std::string> problems;
    project::VerifyAssets(db, problems);
    std::vector<std::string> onDisk;
    if (!ScanLayoutFiles(db.root, onDisk, error)) {
        log.Error("Project folder check failed: " + error);
        return false;
    }
    for (const std::string &file : onDisk)
        if (!project::FindAsset(db, file)) problems.push_back("not in the database: " + file);
    if (logDiskState)
        for (const std::string &p : problems) log.Error("Database check: " + p);

    const bool sameRoot = open_ && db.root == project_.root;
    if (!sameRoot) {
        expanded_.clear();
        for (const char *folder : project::kLayoutFolders) expanded_.insert(folder);
        selected_.clear();
    }
    log.Info("Project \"" + db.manifest.name + "\": " + std::to_string(db.assets.size()) + " assets");
    project_ = std::move(db);
    tree_ = std::move(tree);
    problems_ = std::move(problems);
    open_ = true;
    return true;
}

}  // namespace jeteditor::app
