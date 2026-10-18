// source/editor/ui/ui.h
//
// Immediate-mode widgets for JetEditor, written here because no GUI toolkit is vendored (Dear ImGui is not on disk).
// A frame is BeginFrame(input, size), then the panels call the widgets (each records quads into the draw list and
// reports its clicks), then EndFrame(). ui_gl.h draws the list. Nothing here touches GL or a window, so the widget
// logic is the same on every platform.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::ui {

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;
    float Right() const { return x + w; }
    float Bottom() const { return y + h; }
    bool Contains(float px, float py) const { return px >= x && px < x + w && py >= y && py < y + h; }
    Rect Inset(float d) const { return Rect{x + d, y + d, w - 2 * d, h - 2 * d}; }
    Rect Intersect(const Rect &o) const;
};

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;
};

// A rasterised ASCII font (FreeType, one pixel size) packed into an alpha atlas.
struct Glyph {
    float u0 = 0, v0 = 0, u1 = 0, v1 = 0;  // atlas coordinates
    float w = 0, h = 0;                    // bitmap size in pixels
    float left = 0, top = 0;               // bitmap offset from the pen position on the baseline
    float advance = 0;
};

class Font {
public:
    static constexpr int kAtlasSize = 512;

    // Loads a TTF/OTF file at the given pixel height. Printable ASCII (32..126) is packed; other bytes draw as '?'.
    bool Load(const std::string &path, int pixelHeight, std::string &error);
    const Glyph &Get(unsigned char c) const;
    // Atlas coordinates of a solid white texel, used for filled rectangles.
    const Glyph &Solid() const { return solid_; }
    float Width(const std::string &text) const;
    float Ascent() const { return ascent_; }
    float LineHeight() const { return lineHeight_; }
    const std::vector<uint8_t> &Atlas() const { return atlas_; }

private:
    Glyph glyphs_[95];
    Glyph solid_;
    std::vector<uint8_t> atlas_;
    float ascent_ = 0, lineHeight_ = 0;
};

struct Vertex {
    float x, y, u, v;
    uint8_t r, g, b, a;
};

// Consecutive triangles that share one clip rectangle and one texture (0 is the font atlas, otherwise an RGBA image).
struct Command {
    Rect clip;
    uint32_t first = 0;
    uint32_t count = 0;
    unsigned texture = 0;
};

// What happened since the last frame, in window pixels (origin top-left).
struct Input {
    float mouseX = 0, mouseY = 0;
    bool mouseDown = false;  // left button held
    bool pressed = false;    // left button went down this frame
    bool released = false;   // left button went up this frame
    float wheel = 0;         // notches, positive is up
    std::string typed;       // printable ASCII typed this frame
    bool enter = false, escape = false, backspace = false, tab = false;
};

// FNV-1a of a label. Widgets are identified by the hash of their label or path.
uint32_t Id(const std::string &text);

class Context {
public:
    explicit Context(const Font &font);

    void BeginFrame(const Input &input, float width, float height);
    void EndFrame();

    // Drawing. Text is positioned by the top of its line box.
    void FillRect(const Rect &r, Color c);
    void Text(float x, float y, const std::string &text, Color c);
    // An RGBA texture (a GL texture name) stretched over r. Its row 0 is the bottom, as in GL, so it is drawn upright.
    void Image(const Rect &r, unsigned glTexture);
    float TextWidth(const std::string &text) const { return font_.Width(text); }
    float LineHeight() const { return font_.LineHeight(); }
    void PushClip(const Rect &r);
    void PopClip();

    // Input. Widgets report nothing while the mouse is blocked (a menu or dialog is open above them).
    void SetMouseBlocked(bool blocked) { mouseBlocked_ = blocked; }
    bool Hovered(const Rect &r) const;
    // True on the frame the left button is released over the same widget it was pressed on.
    bool Clickable(uint32_t id, const Rect &r);
    bool Button(uint32_t id, const Rect &r, const std::string &label);
    // A press on r gives the mouse to id until the release. Returns the horizontal movement of the mouse since the last
    // frame while id holds it, 0 otherwise.
    float DragDelta(uint32_t id, const Rect &r);
    // Single-line text field. Typing goes to the focused field; a click on the field focuses it.
    void TextField(uint32_t id, const Rect &r, std::string &text);
    void Focus(uint32_t id) { focus_ = id; }
    bool Focused(uint32_t id) const { return focus_ == id; }
    // Wheel scrolling over r. Returns the offset clamped to the content height.
    float Scroll(const Rect &r, float offset, float contentHeight) const;

    const Input &GetInput() const { return input_; }
    float Width() const { return width_; }
    float Height() const { return height_; }
    const std::vector<Vertex> &Vertices() const { return vertices_; }
    const std::vector<Command> &Commands() const { return commands_; }

private:
    void Emit(const Vertex *v, int count, unsigned texture = 0);
    void Quad(const Rect &r, float u0, float v0, float u1, float v1, Color c, unsigned texture = 0);

    const Font &font_;
    Input input_;
    float width_ = 0, height_ = 0;
    bool mouseBlocked_ = false;
    uint32_t active_ = 0;   // widget that received the press
    uint32_t focus_ = 0;    // focused text field
    float dragX_ = 0;       // mouse x at the last DragDelta frame
    std::vector<Rect> clips_;
    std::vector<Vertex> vertices_;
    std::vector<Command> commands_;
};

}  // namespace jeteditor::ui
