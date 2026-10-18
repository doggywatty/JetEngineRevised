// source/editor/app/model.h
//
// The editor's state and commands, with no window and no GL: the open project (M2), the explorer tree built from its
// asset database, the selection and the log. The GUI (gui.cpp) and the headless --selftest (selftest.cpp) both use it,
// so a command does the same thing either way.
#pragma once

#include "editor/project/project.h"

#include <set>
#include <string>
#include <vector>

namespace jeteditor::app {

struct LogLine {
    bool error = false;
    std::string text;
};

class Log {
public:
    void Info(const std::string &text) { lines_.push_back(LogLine{false, text}); }
    void Error(const std::string &text) { lines_.push_back(LogLine{true, text}); }
    const std::vector<LogLine> &Lines() const { return lines_; }

private:
    std::vector<LogLine> lines_;
};

// A node of the explorer tree. The root is the project; its children are the six layout folders and whatever the
// asset paths put under them. An asset is a node with no children and a path; a folder has folder == true.
struct TreeNode {
    std::string name;
    std::string path;  // project-relative with '/'; "" for the root
    bool folder = false;
    std::vector<TreeNode> children;
};

// Builds the tree from the database. Children are sorted by name with folders before assets. Fails if one path
// would be both a folder and an asset.
bool BuildTree(const project::Project &project, TreeNode &out, std::string &error);

// The asset paths under node, depth-first in tree order.
void CollectAssets(const TreeNode &node, std::vector<std::string> &out);

struct Row {
    const TreeNode *node = nullptr;
    int depth = 0;
};

// The rows the explorer shows: the root's children, and the children of each expanded folder below them.
void VisibleRows(const TreeNode &root, const std::set<std::string> &expanded, std::vector<Row> &rows);

// Every regular file under the six layout folders, as project paths, except the database itself.
bool ScanLayoutFiles(const std::string &root, std::vector<std::string> &out, std::string &error);

// One project at a time. Each command logs what it did, or the error that stopped it.
class Editor {
public:
    Log log;

    bool NewProject(const std::string &root, const std::string &name);
    bool OpenProject(const std::string &root);
    // Imports one file with the M2 importer. Refused when no project is open.
    bool ImportAsset(const std::string &source);

    bool IsOpen() const { return open_; }
    const project::Project &Current() const { return project_; }
    const TreeNode &Tree() const { return tree_; }
    std::set<std::string> &Expanded() { return expanded_; }
    const std::string &Selected() const { return selected_; }
    void Select(const std::string &path) { selected_ = path; }
    // The record of the selected path when that path is an asset, otherwise null.
    const project::AssetRecord *SelectedAsset() const;
    // Disk problems as of the last open or import, as "missing: <path>", "size differs: <path> ...",
    // "hash differs: <path>" or "not in the database: <path>".
    const std::vector<std::string> &DiskProblems() const { return problems_; }

private:
    bool Adopt(project::Project db, bool logDiskState);

    project::Project project_;
    TreeNode tree_;
    std::set<std::string> expanded_;
    std::string selected_;
    bool open_ = false;
    std::vector<std::string> problems_;
};

}  // namespace jeteditor::app
