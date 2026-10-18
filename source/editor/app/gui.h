// source/editor/app/gui.h
#pragma once

#include "editor/app/model.h"
#include "editor/ui/ui.h"
#include "editor/world/world.h"

#include <string>

namespace jet::video {
class Display;
}

namespace jeteditor::app {

// The editor's panels and menu. Each frame draws the menu bar, the project explorer, the world viewport, the asset
// inspector, the log and any open dialog, and runs the commands the user chose. All state lives in the Editor and the
// World. glDisplay is the editor window's display, which the world draws with.
class Gui {
public:
    Gui(Editor &editor, world::World &world, jet::video::Display *glDisplay);

    void Frame(ui::Context &ctx);
    bool QuitRequested() const { return quit_; }
    // Opens the MinionRush data folder in the viewport. Logs the result; on failure the error is also returned.
    bool OpenWorld(const std::string &dataDir, std::string &error);
    // Selects the level entity with this index once the level is loaded (the --select option)
    void SelectWhenLoaded(int index) { pendingSelect_ = index; }

private:
    enum class Dialog { None, NewProject, OpenProject, ImportAsset, OpenWorld };

    void Bar(ui::Context &ctx);
    void Explorer(ui::Context &ctx, const ui::Rect &r);
    void Entities(ui::Context &ctx, const ui::Rect &r);
    void Viewport(ui::Context &ctx, const ui::Rect &r);
    void ViewportInput(ui::Context &ctx, const ui::Rect &body, const ui::Rect &frame);
    void SelectionBox(ui::Context &ctx, const ui::Rect &frame);
    void Inspector(ui::Context &ctx, const ui::Rect &r);
    void EntityInspector(ui::Context &ctx, float labelX, float valueX, float &y);
    // Logs the level once it is loaded: how many entities are drawn, which are runtime-generated, and why the rest are not
    void LogLevel();
    void LogPanel(ui::Context &ctx, const ui::Rect &r);
    void MenuPopup(ui::Context &ctx);
    void DialogBox(ui::Context &ctx);
    void OpenDialog(Dialog which, const std::string &first, const std::string &second);
    void Submit();
    // A dialog's path as typed: a relative one is taken from the folder the editor was started in, since opening a
    // world changes the working directory to the data folder
    std::string Absolute(const std::string &path) const;

    Editor &ed_;
    world::World &world_;
    jet::video::Display *glDisplay_;
    std::string startDir_;
    Dialog dialog_ = Dialog::None;
    bool menuOpen_ = false;
    bool quit_ = false;
    bool focusFirstField_ = false;
    std::string field0_, field1_;
    ui::Rect fileButton_;
    float explorerScroll_ = 0;
    float entityScroll_ = 0;
    float logScroll_ = 0;
    size_t lastLogCount_ = 0;
    // the viewport's drag: the mouse position of the last frame it was held
    bool orbiting_ = false;
    float lastMouseX_ = 0, lastMouseY_ = 0;
    // a press in the viewport is a click (picks an entity) when the mouse does not move before the release
    float pressX_ = 0, pressY_ = 0;
    bool pressMoved_ = false;
    // an inspector drag changed a position since the last release (the release logs it)
    bool positionDragged_ = false;
    bool loggedLevel_ = false;
    int pendingSelect_ = -1;
};

}  // namespace jeteditor::app
