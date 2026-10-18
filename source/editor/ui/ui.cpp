// source/editor/ui/ui.cpp
#include "editor/ui/ui.h"

#include <algorithm>

namespace jeteditor::ui {

namespace {

constexpr float kWheelStep = 40.0f;  // pixels per wheel notch

const Color kButtonFill{58, 62, 70, 255};
const Color kButtonHover{72, 78, 90, 255};
const Color kButtonDown{38, 42, 50, 255};
const Color kButtonEdge{92, 98, 110, 255};
const Color kLabel{230, 232, 236, 255};
const Color kFieldFill{22, 24, 28, 255};
const Color kFieldEdge{90, 96, 108, 255};
const Color kFieldFocusEdge{86, 140, 220, 255};
const Color kCaret{230, 232, 236, 255};

}  // namespace

Rect Rect::Intersect(const Rect &o) const {
    const float x0 = std::max(x, o.x), y0 = std::max(y, o.y);
    const float x1 = std::min(Right(), o.Right()), y1 = std::min(Bottom(), o.Bottom());
    return Rect{x0, y0, std::max(0.0f, x1 - x0), std::max(0.0f, y1 - y0)};
}

uint32_t Id(const std::string &text) {
    uint32_t h = 2166136261u;
    for (unsigned char c : text) {
        h ^= c;
        h *= 16777619u;
    }
    return h == 0 ? 1 : h;
}

Context::Context(const Font &font) : font_(font) {}

void Context::BeginFrame(const Input &input, float width, float height) {
    input_ = input;
    width_ = width;
    height_ = height;
    mouseBlocked_ = false;
    clips_.assign(1, Rect{0, 0, width, height});
    vertices_.clear();
    commands_.clear();
}

void Context::EndFrame() {
    // A release ends the press whether or not a widget claimed it, so no widget stays active
    if (input_.released) active_ = 0;
}

void Context::Emit(const Vertex *v, int count, unsigned texture) {
    const Rect &clip = clips_.back();
    const uint32_t first = static_cast<uint32_t>(vertices_.size());
    vertices_.insert(vertices_.end(), v, v + count);
    if (!commands_.empty()) {
        Command &last = commands_.back();
        const Rect &c = last.clip;
        if (c.x == clip.x && c.y == clip.y && c.w == clip.w && c.h == clip.h && last.first + last.count == first &&
            last.texture == texture) {
            last.count += count;
            return;
        }
    }
    commands_.push_back(Command{clip, first, static_cast<uint32_t>(count), texture});
}

void Context::Quad(const Rect &r, float u0, float v0, float u1, float v1, Color c, unsigned texture) {
    const float x0 = r.x, y0 = r.y, x1 = r.x + r.w, y1 = r.y + r.h;
    const Vertex v[6] = {
        {x0, y0, u0, v0, c.r, c.g, c.b, c.a}, {x1, y0, u1, v0, c.r, c.g, c.b, c.a},
        {x1, y1, u1, v1, c.r, c.g, c.b, c.a}, {x0, y0, u0, v0, c.r, c.g, c.b, c.a},
        {x1, y1, u1, v1, c.r, c.g, c.b, c.a}, {x0, y1, u0, v1, c.r, c.g, c.b, c.a},
    };
    Emit(v, 6, texture);
}

void Context::FillRect(const Rect &r, Color c) {
    const Glyph &s = font_.Solid();
    Quad(r, s.u0, s.v0, s.u1, s.v1, c);
}

void Context::Image(const Rect &r, unsigned glTexture) {
    if (glTexture == 0) return;
    // Top-left of the rectangle takes the texture's top row (v = 1), so the GL image comes out upright
    Quad(r, 0, 1, 1, 0, Color{255, 255, 255, 255}, glTexture);
}

void Context::Text(float x, float y, const std::string &text, Color c) {
    const float baseline = y + font_.Ascent();
    float pen = x;
    for (unsigned char ch : text) {
        const Glyph &g = font_.Get(ch);
        if (g.w > 0 && g.h > 0)
            Quad(Rect{pen + g.left, baseline - g.top, g.w, g.h}, g.u0, g.v0, g.u1, g.v1, c);
        pen += g.advance;
    }
}

void Context::PushClip(const Rect &r) { clips_.push_back(clips_.back().Intersect(r)); }

void Context::PopClip() {
    if (clips_.size() > 1) clips_.pop_back();
}

bool Context::Hovered(const Rect &r) const {
    const float mx = input_.mouseX, my = input_.mouseY;
    return !mouseBlocked_ && r.Contains(mx, my) && clips_.back().Contains(mx, my);
}

bool Context::Clickable(uint32_t id, const Rect &r) {
    const bool hot = Hovered(r);
    if (hot && input_.pressed) active_ = id;
    return input_.released && active_ == id && hot;
}

bool Context::Button(uint32_t id, const Rect &r, const std::string &label) {
    const bool hot = Hovered(r);
    const bool down = hot && active_ == id && input_.mouseDown;
    const bool clicked = Clickable(id, r);
    FillRect(r, kButtonEdge);
    FillRect(r.Inset(1), down ? kButtonDown : hot ? kButtonHover : kButtonFill);
    const float tx = r.x + (r.w - TextWidth(label)) / 2;
    const float ty = r.y + (r.h - font_.LineHeight()) / 2;
    Text(tx, ty, label, kLabel);
    return clicked;
}

float Context::DragDelta(uint32_t id, const Rect &r) {
    if (Hovered(r) && input_.pressed) {
        active_ = id;
        dragX_ = input_.mouseX;
    }
    if (active_ != id || !input_.mouseDown) return 0;
    const float dx = input_.mouseX - dragX_;
    dragX_ = input_.mouseX;
    return dx;
}

void Context::TextField(uint32_t id, const Rect &r, std::string &text) {
    if (Hovered(r) && input_.pressed) focus_ = id;
    const bool focused = focus_ == id;
    if (focused) {
        text += input_.typed;
        if (input_.backspace && !text.empty()) text.pop_back();
    }
    FillRect(r, focused ? kFieldFocusEdge : kFieldEdge);
    FillRect(r.Inset(1), kFieldFill);
    PushClip(r.Inset(4));
    const float ty = r.y + (r.h - font_.LineHeight()) / 2;
    Text(r.x + 6, ty, text, kLabel);
    if (focused) {
        const float cx = r.x + 6 + TextWidth(text) + 1;
        FillRect(Rect{cx, ty, 1, font_.LineHeight()}, kCaret);
    }
    PopClip();
}

float Context::Scroll(const Rect &r, float offset, float contentHeight) const {
    if (Hovered(r) && input_.wheel != 0) offset -= input_.wheel * kWheelStep;
    const float maxOffset = std::max(0.0f, contentHeight - r.h);
    return std::min(std::max(offset, 0.0f), maxOffset);
}

}  // namespace jeteditor::ui
