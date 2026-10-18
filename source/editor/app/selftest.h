// source/editor/app/selftest.h
#pragma once

#include <string>

namespace jeteditor::app {

// jeteditor --selftest <project>: no window. Opens the project, builds the explorer tree, compares the tree with the
// asset database and the files on disk, selects the first asset in tree order and checks the inspector fields.
// Prints one line per check. Returns 0 only if every check passes.
int RunSelfTest(const std::string &root);

}  // namespace jeteditor::app
