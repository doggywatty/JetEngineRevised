// source/editor/app/main.cpp
//
// jeteditor [--font <ttf>] [--world <dataDir>] [--select <n>] [--shot <file.ppm>] [project]
//                                        window; the level (MinionRush data folder) in edit mode in the viewport
// jeteditor --world-edit <dataDir>       headless: loads the level in edit mode and prints its entity count
// jeteditor --selftest <project>         headless check of the project (see selftest.cpp)
// jeteditor --new <folder> <name>        headless File > New project
// jeteditor --import <project> <file>    headless Import asset
//
// --select picks the level entity with that index (from 1, as the inspector counts) once the level is loaded. --shot
// writes the window's contents to a PPM file a few frames after that, then exits.
//
// The window needs an X server. The headless forms need none and print the log they wrote.
#include "editor/app/gui.h"
#include "editor/app/model.h"
#include "editor/app/platform.h"
#include "editor/app/selftest.h"
#include "editor/app/window.h"
#include "editor/ui/ui.h"
#include "editor/ui/ui_gl.h"
#include "editor/world/world.h"
#include "video/displaymode.h"

#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#define usleep(us) Sleep((us) / 1000)
#endif
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

using namespace jeteditor;

namespace {

// The editor's own font is a system font: the game's fallback font (data/droidsansfallback.ttf) is empty in the repo
const char *const kFontCandidates[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/Library/Fonts/Arial.ttf",
};
constexpr int kFontPixels = 14;

void Usage() {
    std::fprintf(stderr,
                 "usage:\n"
                 "  jeteditor [--font <ttf>] [--world <dataDir>] [--select <n>] [--shot <file.ppm>] [project]\n"
                 "  jeteditor --world-edit <dataDir>\n"
                 "  jeteditor --selftest <project>\n"
                 "  jeteditor --new <folder> <name>\n"
                 "  jeteditor --import <project> <file>\n");
}

// The path as an absolute one: the world changes the working directory, so paths given on the command line are
// fixed before that
std::string AbsolutePath(const std::string &path) {
    std::error_code ec;
    const std::string abs = std::filesystem::absolute(path, ec).lexically_normal().string();
    return ec ? path : abs;
}

void PrintLog(const app::Log &log) {
    for (const app::LogLine &line : log.Lines())
        std::printf("%s %s\n", line.error ? "error:" : "info: ", line.text.c_str());
}

int HeadlessNew(const std::string &folder, const std::string &name) {
    app::Editor ed;
    const bool ok = ed.NewProject(folder, name);
    PrintLog(ed.log);
    return ok ? 0 : 1;
}

int HeadlessImport(const std::string &root, const std::string &file) {
    app::Editor ed;
    const bool ok = ed.OpenProject(root) && ed.ImportAsset(file);
    PrintLog(ed.log);
    return ok ? 0 : 1;
}

// The level in edit mode with no window. The GL context is EGL without a surface, as the game's own headless runs
// (tests/test_intro.cpp) use. The level loads, the menu state is dropped, and a few edit frames are drawn. Exits 0 when
// the level is loaded, has entities, and no game state is running.
int HeadlessWorldEdit(const std::string &dataDir) {
    jet::video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(world::World::kWidth, world::World::kHeight);
    app::PlatformDisplay display;
    if (!display.Init(mode)) {
        std::fprintf(stderr, "jeteditor: no GL context for the world\n");
        return 1;
    }
    world::World world;
    std::string error;
    if (!world.Open(dataDir, &display, error)) {
        std::fprintf(stderr, "jeteditor: %s\n", error.c_str());
        return 1;
    }
    constexpr int kMaxLoadFrames = 200000;
    int frames = 0;
    while (!world.IsLoaded() && frames < kMaxLoadFrames) {
        world.Update();
        ++frames;
    }
    if (!world.IsLoaded()) {
        std::fprintf(stderr, "jeteditor: the level did not load in %d frames (%s)\n", frames, world.StateName().c_str());
        world.Close();
        return 1;
    }
    for (int i = 0; i < 3; ++i) world.Update();  // edit frames: the level is drawn, nothing runs

    size_t placed = 0;
    for (const world::LevelEntity &e : world.Entities()) placed += e.placed ? 1 : 0;
    const std::string state = world.StateName();
    std::printf("world-edit: %zu entities in levels/level.bclara (%zu placed)\n", world.Entities().size(), placed);
    std::printf("world-edit: %s, loaded in %d frames, %zu drawn\n", state.c_str(), frames, world.DrawnCount());
    size_t index = 0;
    for (const world::LevelEntity &e : world.Entities()) {
        std::printf("entity %zu  %s  %s  %.3f %.3f %.3f%s  %s\n", ++index, e.name.empty() ? "(unnamed)" : e.name.c_str(),
                    e.type.c_str(), e.x, e.y, e.z, e.placed ? "" : "  (not placed)", e.contentNote.c_str());
    }
    const bool ok = !world.Entities().empty() && state == "static scene, no game state";
    world.Close();
    return ok ? 0 : 1;
}

// Writes the frame as a binary PPM (P6), top row first
bool WritePpm(const std::string &path, int w, int h, const std::vector<uint8_t> &rgb, std::string &error) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        error = "cannot write " + path;
        return false;
    }
    out << "P6\n" << w << " " << h << "\n255\n";
    out.write(reinterpret_cast<const char *>(rgb.data()), static_cast<std::streamsize>(rgb.size()));
    return static_cast<bool>(out);
}

int RunWindow(const std::string &fontPath, const std::string &projectRoot, const std::string &worldDir, int selectIndex,
              const std::string &shotPath) {
    ui::Font font;
    std::string error;
    bool fontOk = false;
    if (!fontPath.empty()) {
        fontOk = font.Load(fontPath, kFontPixels, error);
    } else {
        for (const char *candidate : kFontCandidates) {
            if (font.Load(candidate, kFontPixels, error)) {
                fontOk = true;
                break;
            }
        }
        if (!fontOk) error = "no font found; pass --font <ttf>";
    }
    if (!fontOk) {
        std::fprintf(stderr, "jeteditor: %s\n", error.c_str());
        return 1;
    }

    app::Window window;
    if (!window.Open("JetEditor", 1280, 800, error)) {
        std::fprintf(stderr, "jeteditor: %s\n", error.c_str());
        return 1;
    }
    ui::GlRenderer renderer;
    if (!renderer.Init(font, error)) {
        std::fprintf(stderr, "jeteditor: %s\n", error.c_str());
        return 1;
    }

    app::Editor editor;
    if (!projectRoot.empty())
        editor.OpenProject(projectRoot);  // its failure is in the log panel
    else
        editor.log.Info("No project open. Use File > New project or File > Open project.");

    world::World world;
    app::Gui gui(editor, world, window.GlDisplay());
    if (selectIndex >= 0) gui.SelectWhenLoaded(selectIndex);
    if (!worldDir.empty()) {
        std::string error;
        if (!gui.OpenWorld(worldDir, error)) {
            std::fprintf(stderr, "jeteditor: %s\n", error.c_str());
            renderer.Shutdown();
            world.Close();
            window.Close();
            return 1;
        }
    }
    ui::Context ctx(font);
    const ui::Color clear{24, 26, 30, 255};
    // --shot: editor frames drawn after the level loads before the window is read back
    constexpr int kShotFramesAfterLoad = 30;
    int shotFrames = kShotFramesAfterLoad;
    while (!gui.QuitRequested()) {
        ui::Input input;
        bool closeRequested = false, anyEvent = false;
        window.Poll(input, closeRequested, anyEvent);
        if (closeRequested) break;
        int w = 0, h = 0;
        window.Size(w, h);
        if (w < 1 || h < 1) {  // minimised or not yet mapped
            usleep(10000);
            continue;
        }
        world.Update();  // one loading step, or the level's edit frame; the viewport's frame is drawn by the panel below
        ctx.BeginFrame(input, static_cast<float>(w), static_cast<float>(h));
        gui.Frame(ctx);
        ctx.EndFrame();
        renderer.SetFramebuffer(window.Framebuffer());  // a resized window on macOS has a new framebuffer
        renderer.Draw(ctx, w, h, clear);
        if (!shotPath.empty() && world.IsLoaded() && --shotFrames <= 0) {
            std::vector<uint8_t> rgb;
            renderer.ReadBack(w, h, rgb);
            const bool written = WritePpm(shotPath, w, h, rgb, error);
            if (!written) std::fprintf(stderr, "jeteditor: %s\n", error.c_str());
            else std::printf("shot: wrote %s (%dx%d) in edit view: %s\n", shotPath.c_str(), w, h, world.StateName().c_str());
            renderer.Shutdown();
            world.Close();
            window.Close();
            return written ? 0 : 1;
        }
        window.Swap();
        // An idle editor redraws about 100 times a second, which is enough for hover and caret updates
        if (!anyEvent) usleep(10000);
    }
    renderer.Shutdown();
    world.Close();
    window.Close();
    return 0;
}

}  // namespace

int main(int argc, char **argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (!args.empty() && args[0] == "--selftest") {
        if (args.size() != 2) {
            Usage();
            return 2;
        }
        return app::RunSelfTest(args[1]);
    }
    if (!args.empty() && args[0] == "--new") {
        if (args.size() != 3) {
            Usage();
            return 2;
        }
        return HeadlessNew(args[1], args[2]);
    }
    if (!args.empty() && args[0] == "--import") {
        if (args.size() != 3) {
            Usage();
            return 2;
        }
        return HeadlessImport(args[1], args[2]);
    }
    if (!args.empty() && args[0] == "--world-edit") {
        if (args.size() != 2) {
            Usage();
            return 2;
        }
        return HeadlessWorldEdit(AbsolutePath(args[1]));
    }

    std::string font, project, worldDir, shotPath;
    int selectIndex = -1;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--font" && i + 1 < args.size()) {
            font = AbsolutePath(args[++i]);
        } else if (args[i] == "--world" && i + 1 < args.size()) {
            worldDir = AbsolutePath(args[++i]);
        } else if (args[i] == "--select" && i + 1 < args.size()) {
            const long n = std::strtol(args[++i].c_str(), nullptr, 10);
            if (n < 1) {
                std::fprintf(stderr, "jeteditor: --select takes an entity number from 1\n");
                return 2;
            }
            selectIndex = static_cast<int>(n - 1);
        } else if (args[i] == "--shot" && i + 1 < args.size()) {
            shotPath = AbsolutePath(args[++i]);
        } else if (args[i].size() > 1 && args[i][0] == '-') {
            Usage();
            return 2;
        } else if (project.empty()) {
            project = AbsolutePath(args[i]);
        } else {
            Usage();
            return 2;
        }
    }
    if ((selectIndex >= 0 || !shotPath.empty()) && worldDir.empty()) {
        std::fprintf(stderr, "jeteditor: --select and --shot need --world <dataDir>\n");
        return 2;
    }
    return RunWindow(font, project, worldDir, selectIndex, shotPath);
}
