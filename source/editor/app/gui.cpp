// source/editor/app/gui.cpp
#include "editor/app/gui.h"

#include "entity.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <system_error>
#include <vector>

namespace jeteditor::app {

namespace fs = std::filesystem;
using ui::Color;
using ui::Id;
using ui::Rect;

namespace {

constexpr float kBarHeight = 26;
constexpr float kExplorerWidth = 300;
constexpr float kInspectorWidth = 300;
constexpr float kHeaderHeight = 24;
constexpr float kRowHeight = 20;
constexpr float kFieldIndent = 8;
// level units per pixel of a drag on a position field
constexpr float kDragStep = 0.01f;

const Color kWindowFill{24, 26, 30, 255};
const Color kPanelFill{30, 32, 37, 255};
const Color kBarFill{36, 38, 44, 255};
const Color kHeaderFill{42, 45, 52, 255};
const Color kSeparator{60, 64, 72, 255};
const Color kSelected{52, 96, 160, 255};
const Color kHover{46, 50, 58, 255};
const Color kText{224, 227, 232, 255};
const Color kDim{140, 146, 156, 255};
const Color kInfo{200, 206, 214, 255};
const Color kError{255, 118, 118, 255};

// The part after "<kind>: " of a disk problem names a path; it is a match when the path ends there or a space follows
bool ProblemIsFor(const std::string &problem, const std::string &path) {
    const size_t colon = problem.find(": ");
    if (colon == std::string::npos) return false;
    const std::string rest = problem.substr(colon + 2);
    if (rest.compare(0, path.size(), path) != 0) return false;
    return rest.size() == path.size() || rest[path.size()] == ' ';
}

std::string BaseName(const std::string &path) {
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

std::string DirName(const std::string &path) {
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? std::string() : path.substr(0, slash);
}

}  // namespace

Gui::Gui(Editor &editor, world::World &world, jet::video::Display *glDisplay)
    : ed_(editor), world_(world), glDisplay_(glDisplay) {
    std::error_code ec;
    startDir_ = fs::current_path(ec).string();
}

bool Gui::OpenWorld(const std::string &dataDir, std::string &error) {
    if (!world_.Open(dataDir, glDisplay_, error)) {
        ed_.log.Error("Open world failed: " + error);
        return false;
    }
    ed_.log.Info("Opened world " + world_.DataDir() + " (MinionRush level in edit mode: no gameplay state runs)");
    return true;
}

void Gui::Frame(ui::Context &ctx) {
    const float w = ctx.Width(), h = ctx.Height();
    const float logH = std::min(320.0f, std::max(120.0f, h * 0.28f));
    const float mainH = h - kBarHeight - logH;
    const float viewportW = std::max(0.0f, w - kExplorerWidth - kInspectorWidth);
    const bool modal = dialog_ != Dialog::None || menuOpen_;
    const Rect viewportR{kExplorerWidth, kBarHeight, viewportW, mainH};
    const Rect inspectorR{kExplorerWidth + viewportW, kBarHeight, w - kExplorerWidth - viewportW, mainH};
    const Rect logR{0, h - logH, w, logH};
    // With a level open, the left column holds the project explorer above the level's entity list
    const float splitH = world_.IsOpen() ? std::floor(mainH * 0.5f) : mainH;
    const Rect explorerR{0, kBarHeight, kExplorerWidth, splitH};
    const Rect entitiesR{0, kBarHeight + splitH, kExplorerWidth, mainH - splitH};

    if (pendingSelect_ >= 0 && world_.IsLoaded()) {
        if (static_cast<size_t>(pendingSelect_) < world_.Entities().size()) {
            world_.Select(pendingSelect_);
            world_.FocusSelected();
            ed_.Select("");
        } else {
            ed_.log.Error("Select failed: the level has " + std::to_string(world_.Entities().size()) + " entities, no index " +
                          std::to_string(pendingSelect_));
        }
        pendingSelect_ = -1;
    }

    if (world_.IsLoaded() && !loggedLevel_) LogLevel();

    Bar(ctx);
    ctx.SetMouseBlocked(modal);
    Explorer(ctx, explorerR);
    if (world_.IsOpen()) Entities(ctx, entitiesR);
    Viewport(ctx, viewportR);
    Inspector(ctx, inspectorR);
    LogPanel(ctx, logR);
    ctx.SetMouseBlocked(false);
    if (menuOpen_) MenuPopup(ctx);
    if (dialog_ != Dialog::None) DialogBox(ctx);
}

void Gui::Bar(ui::Context &ctx) {
    ctx.FillRect(Rect{0, 0, ctx.Width(), kBarHeight}, kBarFill);
    ctx.FillRect(Rect{0, kBarHeight - 1, ctx.Width(), 1}, kSeparator);
    // Only an open dialog stops the File button; an open menu is closed by clicking the button again
    ctx.SetMouseBlocked(dialog_ != Dialog::None);
    fileButton_ = Rect{4, 3, 56, 20};
    if (ctx.Button(Id("menu:file"), fileButton_, "File")) menuOpen_ = !menuOpen_;
    ctx.SetMouseBlocked(false);

    std::string title = "JetEditor";
    if (ed_.IsOpen()) title += "  -  " + ed_.Current().manifest.name + "  (" + ed_.Current().root + ")";
    ctx.Text(72, 5, title, kText);
}

void Gui::Explorer(ui::Context &ctx, const Rect &r) {
    ctx.FillRect(r, kPanelFill);
    const Rect head{r.x, r.y, r.w, kHeaderHeight};
    ctx.FillRect(head, kHeaderFill);
    ctx.Text(r.x + 8, r.y + 5, ed_.IsOpen() ? "Explorer: " + ed_.Current().manifest.name : "Explorer", kText);
    ctx.FillRect(Rect{r.Right() - 1, r.y, 1, r.h}, kSeparator);

    const Rect list{r.x, r.y + kHeaderHeight, r.w - 1, r.h - kHeaderHeight};
    if (!ed_.IsOpen()) {
        ctx.Text(list.x + kFieldIndent, list.y + 8, "No project open.", kDim);
        ctx.Text(list.x + kFieldIndent, list.y + 26, "Use File > New project", kDim);
        ctx.Text(list.x + kFieldIndent, list.y + 44, "or File > Open project.", kDim);
        return;
    }
    std::vector<Row> rows;
    VisibleRows(ed_.Tree(), ed_.Expanded(), rows);
    explorerScroll_ = ctx.Scroll(list, explorerScroll_, static_cast<float>(rows.size()) * kRowHeight);

    ctx.PushClip(list);
    for (size_t i = 0; i < rows.size(); ++i) {
        const Rect rr{list.x, list.y + static_cast<float>(i) * kRowHeight - explorerScroll_, list.w, kRowHeight};
        if (rr.Bottom() < list.y || rr.y > list.Bottom()) continue;
        const TreeNode &node = *rows[i].node;
        const bool selected = node.path == ed_.Selected();
        if (selected)
            ctx.FillRect(rr, kSelected);
        else if (ctx.Hovered(rr))
            ctx.FillRect(rr, kHover);

        float x = rr.x + kFieldIndent + static_cast<float>(rows[i].depth) * 16;
        if (node.folder) {
            const bool open = ed_.Expanded().count(node.path) != 0;
            ctx.Text(x, rr.y + 3, open ? "-" : "+", kDim);
            x += 14;
        }
        ctx.Text(x, rr.y + 3, node.name, node.folder ? kText : kInfo);

        if (ctx.Clickable(Id("tree:" + node.path), rr)) {
            ed_.Select(node.path);
            world_.Select(-1);
            if (node.folder) {
                std::set<std::string> &expanded = ed_.Expanded();
                if (!expanded.erase(node.path)) expanded.insert(node.path);
            }
        }
    }
    ctx.PopClip();
}

// The level's entities as the level file lists them. A click selects one: the inspector shows it and the viewport marks it.
void Gui::Entities(ui::Context &ctx, const Rect &r) {
    ctx.FillRect(r, kPanelFill);
    ctx.FillRect(Rect{r.x, r.y, r.w, kHeaderHeight}, kHeaderFill);
    const std::vector<world::LevelEntity> &entities = world_.Entities();
    std::string title = "Entities";
    if (world_.IsLoaded()) title += " (" + std::to_string(entities.size()) + ")";
    ctx.Text(r.x + 8, r.y + 5, title, kText);
    ctx.FillRect(Rect{r.Right() - 1, r.y, 1, r.h}, kSeparator);

    const Rect list{r.x, r.y + kHeaderHeight, r.w - 1, r.h - kHeaderHeight};
    if (!world_.IsLoaded()) {
        ctx.Text(list.x + kFieldIndent, list.y + 8, "Loading the level...", kDim);
        return;
    }
    entityScroll_ = ctx.Scroll(list, entityScroll_, static_cast<float>(entities.size()) * kRowHeight);

    ctx.PushClip(list);
    for (size_t i = 0; i < entities.size(); ++i) {
        const Rect rr{list.x, list.y + static_cast<float>(i) * kRowHeight - entityScroll_, list.w, kRowHeight};
        if (rr.Bottom() < list.y || rr.y > list.Bottom()) continue;
        const bool selected = world_.Selected() == static_cast<int>(i);
        if (selected)
            ctx.FillRect(rr, kSelected);
        else if (ctx.Hovered(rr))
            ctx.FillRect(rr, kHover);

        const world::LevelEntity &e = entities[i];
        // the drawn entities are bright; the ones the view does not draw are dim, and the inspector says why
        const Color nameColor = e.name.empty() ? kDim : e.content == world::Content::Model ? kText : kDim;
        ctx.Text(rr.x + kFieldIndent, rr.y + 3, e.name.empty() ? std::string("(unnamed)") : e.name, nameColor);
        ctx.Text(rr.x + 160, rr.y + 3, e.type, kDim);

        if (ctx.Clickable(Id("entity:" + std::to_string(i)), rr)) {
            world_.Select(static_cast<int>(i));
            world_.FocusSelected();
            ed_.Select("");
        }
    }
    ctx.PopClip();
}

// The world's frame, scaled to fit the panel and centred on black. The game draws the frame; a marker is drawn over
// the selected entity only.
void Gui::Viewport(ui::Context &ctx, const Rect &r) {
    ctx.FillRect(r, kPanelFill);
    ctx.FillRect(Rect{r.x, r.y, r.w, kHeaderHeight}, kHeaderFill);
    std::string title = "Viewport";
    if (world_.IsOpen()) title += ": " + world_.StateName() + ", frame " + std::to_string(world_.Frames());
    ctx.Text(r.x + 8, r.y + 5, title, kText);
    ctx.FillRect(Rect{r.Right() - 1, r.y, 1, r.h}, kSeparator);

    const Rect body{r.x, r.y + kHeaderHeight, r.w - 1, r.h - kHeaderHeight};
    ctx.FillRect(body, Color{0, 0, 0, 255});
    if (!world_.IsOpen()) {
        ctx.Text(body.x + kFieldIndent + 4, body.y + 8, "No world open.", kDim);
        ctx.Text(body.x + kFieldIndent + 4, body.y + 26, "Use File > Open world, or start with --world <dataDir>.", kDim);
        return;
    }
    const float frameW = static_cast<float>(world::World::kWidth), frameH = static_cast<float>(world::World::kHeight);
    const float scale = std::min(body.w / frameW, body.h / frameH);
    if (scale <= 0) return;
    const Rect frame{body.x + (body.w - frameW * scale) / 2, body.y + (body.h - frameH * scale) / 2, frameW * scale,
                     frameH * scale};
    ViewportInput(ctx, body, frame);
    ctx.PushClip(body);
    ctx.Image(frame, world_.Texture());
    if (world_.IsLoaded()) SelectionBox(ctx, frame);
    ctx.PopClip();
}

// Drag with the left button to orbit, wheel to zoom. A press and release without a move picks the drawn entity under the
// mouse (the nearest one whose box is there), or clears the selection over nothing. Only the viewport's own area takes
// the input.
void Gui::ViewportInput(ui::Context &ctx, const Rect &body, const Rect &frame) {
    const ui::Input &in = ctx.GetInput();
    if (!world_.IsLoaded()) return;
    const bool wasOrbiting = orbiting_;
    if (in.pressed && ctx.Hovered(body)) {
        orbiting_ = true;
        pressMoved_ = false;
        pressX_ = in.mouseX;
        pressY_ = in.mouseY;
        lastMouseX_ = in.mouseX;
        lastMouseY_ = in.mouseY;
    }
    if (!in.mouseDown) orbiting_ = false;
    if (orbiting_) {
        if (std::fabs(in.mouseX - pressX_) + std::fabs(in.mouseY - pressY_) > 4) pressMoved_ = true;
        world_.Orbit(in.mouseX - lastMouseX_, in.mouseY - lastMouseY_);
        lastMouseX_ = in.mouseX;
        lastMouseY_ = in.mouseY;
    }
    if (wasOrbiting && in.released && !pressMoved_) {
        const float fx = (in.mouseX - frame.x) * static_cast<float>(world::World::kWidth) / frame.w;
        const float fy = (in.mouseY - frame.y) * static_cast<float>(world::World::kHeight) / frame.h;
        const int hit = world_.PickAt(fx, fy);
        world_.Select(hit);
        if (hit >= 0) ed_.Select("");
    }
    if (in.wheel != 0 && ctx.Hovered(body)) world_.Zoom(in.wheel);
}

// Brackets around the selected entity's box, projected into the frame. Only a drawn entity has a box.
void Gui::SelectionBox(ui::Context &ctx, const Rect &frame) {
    float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    if (!world_.SelectedScreenBox(x0, y0, x1, y1)) return;
    const float sx = frame.w / static_cast<float>(world::World::kWidth);
    const float sy = frame.h / static_cast<float>(world::World::kHeight);
    const Rect box{frame.x + x0 * sx, frame.y + y0 * sy, (x1 - x0) * sx, (y1 - y0) * sy};
    const Color mark{255, 190, 60, 255};
    const float t = 2;
    ctx.FillRect(Rect{box.x, box.y, box.w, t}, mark);
    ctx.FillRect(Rect{box.x, box.Bottom() - t, box.w, t}, mark);
    ctx.FillRect(Rect{box.x, box.y, t, box.h}, mark);
    ctx.FillRect(Rect{box.Right() - t, box.y, t, box.h}, mark);
}

void Gui::Inspector(ui::Context &ctx, const Rect &r) {
    ctx.FillRect(r, kPanelFill);
    ctx.FillRect(Rect{r.x, r.y, r.w, kHeaderHeight}, kHeaderFill);
    ctx.Text(r.x + 8, r.y + 5, "Inspector", kText);
    const Rect body{r.x, r.y + kHeaderHeight, r.w, r.h - kHeaderHeight};
    ctx.PushClip(body);

    const float labelX = body.x + 12, valueX = body.x + 120;
    float y = body.y + 10;
    auto field = [&](const std::string &label, const std::string &value) {
        ctx.Text(labelX, y, label, kDim);
        ctx.Text(valueX, y, value, kInfo);
        y += 22;
    };

    if (world_.IsLoaded() && world_.Selected() >= 0) {
        EntityInspector(ctx, labelX, valueX, y);
    } else if (!ed_.IsOpen()) {
        ctx.Text(labelX, y, "Open a project to inspect its assets.", kDim);
    } else if (const project::AssetRecord *a = ed_.SelectedAsset()) {
        field("Name", BaseName(a->path));
        field("Type", a->type);
        field("Path", a->path);
        field("Folder", DirName(a->path));
        field("Hash", a->hash);
        field("Size", std::to_string(a->size) + " bytes");
        field("Source", a->source);
        std::string disk = "ok: matches the database";
        for (const std::string &problem : ed_.DiskProblems())
            if (ProblemIsFor(problem, a->path)) disk = problem;
        ctx.Text(labelX, y, "Disk", kDim);
        ctx.Text(valueX, y, disk, disk.compare(0, 2, "ok") == 0 ? kInfo : kError);
    } else if (!ed_.Selected().empty()) {
        const std::string prefix = ed_.Selected() + "/";
        size_t count = 0;
        for (const project::AssetRecord &a : ed_.Current().assets)
            if (a.path.compare(0, prefix.size(), prefix) == 0) ++count;
        field("Folder", ed_.Selected());
        field("Assets", std::to_string(count));
    } else {
        ctx.Text(labelX, y, "Select an asset or folder in the explorer.", kDim);
    }
    ctx.PopClip();
}

// The selected level entity, as the level file gives it: its name, its template and its placed position
void Gui::EntityInspector(ui::Context &ctx, float labelX, float valueX, float &y) {
    const std::vector<world::LevelEntity> &entities = world_.Entities();
    const size_t index = static_cast<size_t>(world_.Selected());
    if (index >= entities.size()) return;
    const world::LevelEntity &e = entities[index];
    auto field = [&](const std::string &label, const std::string &value) {
        ctx.Text(labelX, y, label, kDim);
        ctx.Text(valueX, y, value, kInfo);
        y += 22;
    };
    field("Entity", e.name.empty() ? std::string("(unnamed)") : e.name);
    field("Type", e.type);
    field("Index", std::to_string(index + 1) + " of " + std::to_string(entities.size()));
    field("Drawn", e.content == world::Content::Model ? "yes" : "no");
    // the note wrapped to the value column
    const float wrapW = 300 - (valueX - labelX) - 12;
    std::string line;
    std::vector<std::string> lines;
    std::string word;
    std::istringstream words(e.contentNote);
    while (words >> word) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && ctx.TextWidth(candidate) > wrapW) {
            lines.push_back(line);
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) lines.push_back(line);
    for (const std::string &l : lines) {
        ctx.Text(valueX, y, l, e.content == world::Content::Model ? kInfo : kDim);
        y += 16;
    }
    y += 6;

    if (!e.placed || !e.source) {
        field("Position", "not placed (no transform)");
        return;
    }
    // Position: drag a value to move the entity along that axis. The change is in memory only.
    static const char *const kAxisNames[] = {"Position X", "Position Y", "Position Z"};
    const float values[] = {e.x, e.y, e.z};
    for (int axis = 0; axis < 3; ++axis) {
        ctx.Text(labelX, y + 3, kAxisNames[axis], kDim);
        const Rect box{valueX, y, 120, 20};
        ctx.FillRect(box, Color{22, 24, 28, 255});
        ctx.FillRect(Rect{box.x, box.Bottom() - 1, box.w, 1}, Color{90, 96, 108, 255});
        char text[32];
        std::snprintf(text, sizeof text, "%.3f", values[axis]);
        ctx.Text(box.x + 6, y + 3, text, kInfo);
        const float dx = ctx.DragDelta(Id("pos:" + std::to_string(axis)), box);
        if (dx != 0) {
            world_.SetSelectedAxis(axis, values[axis] + dx * kDragStep);
            positionDragged_ = true;
        }
        y += 24;
    }
    if (positionDragged_ && ctx.GetInput().released) {
        const world::LevelEntity &moved = world_.Entities()[index];
        ed_.log.Info("Moved " + (moved.name.empty() ? moved.type : moved.name) + " in memory to (" + std::to_string(moved.x) +
                     ", " + std::to_string(moved.y) + ", " + std::to_string(moved.z) + "). Not saved.");
        positionDragged_ = false;
    }

    const math::quat<float> &rot = e.source->GetRotation();
    const math::vec3<float> &scale = e.source->GetScale();
    char rs[64];
    std::snprintf(rs, sizeof rs, "%.2f %.2f %.2f %.2f", rot.x, rot.y, rot.z, rot.w);
    field("Rotation xyzw", rs);
    char ss[64];
    std::snprintf(ss, sizeof ss, "%.2f %.2f %.2f", scale.x, scale.y, scale.z);
    field("Scale", ss);
    if (e.content == world::Content::Model) field("Model", e.model);
}

// The level as the editor reads it: what is drawn, what is runtime-generated, and what has no visual content
void Gui::LogLevel() {
    loggedLevel_ = true;
    const std::vector<world::LevelEntity> &entities = world_.Entities();
    size_t runtime = 0, nonVisual = 0;
    for (const world::LevelEntity &e : entities) {
        if (e.content == world::Content::Runtime) ++runtime;
        if (e.content == world::Content::NonVisual) ++nonVisual;
    }
    ed_.log.Info("Level: " + std::to_string(entities.size()) + " entities; " + std::to_string(world_.DrawnCount()) +
                 " drawn, " + std::to_string(runtime) + " not drawn as runtime content (track templates or unported classes), " +
                 std::to_string(nonVisual) + " with no visual content (not drawn).");
    for (const world::LevelEntity &e : entities)
        if (e.content == world::Content::Runtime)
            ed_.log.Info("Not drawn, runtime content: " + (e.name.empty() ? std::string("(unnamed)") : e.name) + " (" + e.type + "). " +
                         e.contentNote);
    if (world_.DrawnCount() == 0)
        ed_.log.Info("No entity of the level file has a placed model, so the view is empty. The track and the decorations are "
                     "in the level's libraries and the level templates build them at gameplay start.");
}

void Gui::LogPanel(ui::Context &ctx, const Rect &r) {
    ctx.FillRect(Rect{r.x, r.y, r.w, 1}, kSeparator);
    ctx.FillRect(Rect{r.x, r.y + 1, r.w, r.h - 1}, kPanelFill);
    const std::vector<LogLine> &lines = ed_.log.Lines();
    ctx.Text(8, r.y + 5, "Log (" + std::to_string(lines.size()) + ")", kText);

    const Rect list{r.x, r.y + kHeaderHeight, r.w, r.h - kHeaderHeight};
    const float lineH = ctx.LineHeight() + 3;
    if (lines.size() != lastLogCount_) {
        // New lines: show the newest. Scroll() clamps the offset to the content.
        lastLogCount_ = lines.size();
        logScroll_ = 1e9f;
    }
    logScroll_ = ctx.Scroll(list, logScroll_, static_cast<float>(lines.size()) * lineH);

    ctx.PushClip(list);
    for (size_t i = 0; i < lines.size(); ++i) {
        const float y = list.y + static_cast<float>(i) * lineH - logScroll_;
        if (y + lineH < list.y || y > list.Bottom()) continue;
        ctx.Text(list.x + kFieldIndent, y + 1, lines[i].text, lines[i].error ? kError : kInfo);
    }
    ctx.PopClip();
}

void Gui::MenuPopup(ui::Context &ctx) {
    static const char *const kItems[] = {"New project...", "Open project...", "Import asset...", "Open world...", "Quit"};
    const size_t itemCount = sizeof kItems / sizeof kItems[0];
    const float itemH = 26;
    const Rect pop{4, kBarHeight + 2, 200, static_cast<float>(itemCount) * itemH + 8};
    const ui::Input &in = ctx.GetInput();
    if (in.pressed && !pop.Contains(in.mouseX, in.mouseY) && !fileButton_.Contains(in.mouseX, in.mouseY)) {
        menuOpen_ = false;
        return;
    }
    ctx.FillRect(pop, kSeparator);
    ctx.FillRect(pop.Inset(1), kPanelFill);
    for (size_t i = 0; i < itemCount; ++i) {
        const Rect item{pop.x + 4, pop.y + 4 + static_cast<float>(i) * itemH, pop.w - 8, itemH - 2};
        if (ctx.Hovered(item)) ctx.FillRect(item, kHover);
        ctx.Text(item.x + 8, item.y + 5, kItems[i], kText);
        if (!ctx.Clickable(Id(std::string("menu:") + kItems[i]), item)) continue;
        menuOpen_ = false;
        switch (i) {
        case 0: {
            const std::string folder = (fs::path(startDir_) / "untitled-project").string();
            OpenDialog(Dialog::NewProject, folder, "Untitled project");
            break;
        }
        case 1:
            OpenDialog(Dialog::OpenProject, std::string(), std::string());
            break;
        case 2:
            if (!ed_.IsOpen())
                ed_.log.Error("Import asset failed: no project is open. Use File > New project or File > Open project first.");
            else
                OpenDialog(Dialog::ImportAsset, std::string(), std::string());
            break;
        case 3:
            OpenDialog(Dialog::OpenWorld, world_.IsOpen() ? world_.DataDir() : std::string(), std::string());
            break;
        default:
            quit_ = true;
            break;
        }
        return;
    }
}

void Gui::OpenDialog(Dialog which, const std::string &first, const std::string &second) {
    dialog_ = which;
    field0_ = first;
    field1_ = second;
    focusFirstField_ = true;
    menuOpen_ = false;
}

void Gui::DialogBox(ui::Context &ctx) {
    const float w = ctx.Width(), h = ctx.Height();
    const bool twoFields = dialog_ == Dialog::NewProject;
    const bool opens = dialog_ == Dialog::OpenProject || dialog_ == Dialog::OpenWorld;
    const char *title = twoFields                          ? "New project"
                        : dialog_ == Dialog::OpenProject   ? "Open project"
                        : dialog_ == Dialog::OpenWorld     ? "Open world"
                                                           : "Import asset";
    const char *firstLabel = dialog_ == Dialog::ImportAsset ? "Source file"
                             : dialog_ == Dialog::OpenWorld ? "Data folder (the one with levels/ and profiles.json)"
                                                            : "Folder";
    const char *action = twoFields ? "Create" : opens ? "Open" : "Import";

    ctx.FillRect(Rect{0, 0, w, h}, Color{0, 0, 0, 150});
    const float boxW = 560, boxH = twoFields ? 200 : 150;
    const Rect box{(w - boxW) / 2, (h - boxH) / 2, boxW, boxH};
    ctx.FillRect(box, kSeparator);
    ctx.FillRect(box.Inset(1), kPanelFill);
    ctx.Text(box.x + 16, box.y + 12, title, kText);

    const ui::Input &in = ctx.GetInput();
    const uint32_t id0 = Id("dlg:0"), id1 = Id("dlg:1");
    if (focusFirstField_) {
        ctx.Focus(id0);
        focusFirstField_ = false;
    }
    if (in.tab && twoFields) ctx.Focus(ctx.Focused(id0) ? id1 : id0);

    ctx.Text(box.x + 16, box.y + 40, firstLabel, kDim);
    const Rect f0{box.x + 16, box.y + 58, box.w - 32, 26};
    ctx.TextField(id0, f0, field0_);
    if (twoFields) {
        ctx.Text(box.x + 16, box.y + 96, "Name", kDim);
        ctx.TextField(id1, Rect{box.x + 16, box.y + 114, box.w - 32, 26}, field1_);
    }

    const float by = box.Bottom() - 38;
    const bool clickedAction = ctx.Button(Id("dlg:ok"), Rect{box.Right() - 196, by, 84, 26}, action);
    const bool clickedCancel = ctx.Button(Id("dlg:cancel"), Rect{box.Right() - 104, by, 88, 26}, "Cancel");
    if (clickedCancel || in.escape) {
        dialog_ = Dialog::None;
    } else if (clickedAction || in.enter) {
        Submit();
    }
}

std::string Gui::Absolute(const std::string &path) const {
    if (path.empty() || fs::path(path).is_absolute()) return path;
    return (fs::path(startDir_) / path).lexically_normal().string();
}

void Gui::Submit() {
    const Dialog which = dialog_;
    dialog_ = Dialog::None;
    switch (which) {
    case Dialog::NewProject:
        if (field0_.empty())
            ed_.log.Error("New project failed: no folder given");
        else if (field1_.empty())
            ed_.log.Error("New project failed: no name given");
        else
            ed_.NewProject(Absolute(field0_), field1_);
        break;
    case Dialog::OpenProject:
        if (field0_.empty())
            ed_.log.Error("Open project failed: no folder given");
        else
            ed_.OpenProject(Absolute(field0_));
        break;
    case Dialog::ImportAsset:
        if (field0_.empty())
            ed_.log.Error("Import asset failed: no file given");
        else
            ed_.ImportAsset(Absolute(field0_));
        break;
    case Dialog::OpenWorld: {
        std::string error;
        if (field0_.empty())
            ed_.log.Error("Open world failed: no folder given");
        else
            OpenWorld(Absolute(field0_), error);
        break;
    }
    case Dialog::None:
        break;
    }
}

}  // namespace jeteditor::app
