// source/editor/app/window.cpp
#include "editor/app/window.h"

// macOS has its own window (window_mac.mm); this file handles Linux (X11), Windows (Win32), and the fallback.

// The engine's headers come first: X11 defines None, Bool and Status, which the engine's enums also use.
#include "video/displaymode.h"
#if defined(__linux__)
#include "video/egl/jetegldisplay.h"
#elif defined(_WIN32)
#include "video/win32display.h"
#endif

#include <cstdint>

#if defined(__linux__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace jeteditor::app {

#if defined(__linux__)

struct Window::Impl {
    ::Display *dpy = nullptr;
    ::Window win = 0;
    Atom wmDelete = 0;
    int width = 0, height = 0;
    float mouseX = 0, mouseY = 0;
    bool mouseDown = false;
    jet::video::JetEGLDisplay *egl = nullptr;
};

Window::Window() : impl_(new Impl) {}

Window::~Window() {
    Close();
    delete impl_;
}

bool Window::Open(const std::string &title, int width, int height, std::string &error) {
    if (impl_->dpy) {
        error = "the window is already open";
        return false;
    }
    ::Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        error = "cannot open an X display: DISPLAY is not set or no X server is running. "
                "--selftest, --new and --import do not need one";
        return false;
    }
    const int screen = DefaultScreen(dpy);
    const unsigned long black = BlackPixel(dpy, screen);
    ::Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 0, 0, static_cast<unsigned>(width),
                                       static_cast<unsigned>(height), 0, black, black);
    XStoreName(dpy, win, title.c_str());
    XSelectInput(dpy, win,
                 StructureNotifyMask | ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask |
                     PointerMotionMask);
    Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wmDelete, 1);
    XMapWindow(dpy, win);
    XFlush(dpy);

    impl_->dpy = dpy;
    impl_->win = win;
    impl_->wmDelete = wmDelete;
    impl_->width = width;
    impl_->height = height;

    // The window's handle is the EGLNativeWindowType on X11. The engine's display chooses the config, makes a GLES 2
    // context current and wraps the surface.
    impl_->egl = new jet::video::JetEGLDisplay(reinterpret_cast<void *>(static_cast<uintptr_t>(win)));
    jet::video::DisplayMode mode;
    mode.size.x = static_cast<unsigned int>(width);
    mode.size.y = static_cast<unsigned int>(height);
    if (!impl_->egl->Init(mode)) {
        error = "EGL setup failed for the editor window (no GLES 2 config matches the window)";
        Close();
        return false;
    }
    return true;
}

void Window::Close() {
    if (impl_->egl) {
        delete impl_->egl;
        impl_->egl = nullptr;
    }
    if (impl_->dpy) {
        XDestroyWindow(impl_->dpy, impl_->win);
        XCloseDisplay(impl_->dpy);
        impl_->dpy = nullptr;
        impl_->win = 0;
    }
}

bool Window::IsOpen() const { return impl_->dpy != nullptr && impl_->egl != nullptr; }

void Window::Poll(ui::Input &input, bool &closeRequested, bool &anyEvent) {
    input.pressed = false;
    input.released = false;
    input.wheel = 0;
    input.typed.clear();
    input.enter = input.escape = input.backspace = input.tab = false;
    closeRequested = false;
    anyEvent = false;
    if (!IsOpen()) return;

    while (XPending(impl_->dpy) > 0) {
        XEvent ev;
        XNextEvent(impl_->dpy, &ev);
        anyEvent = true;
        switch (ev.type) {
        case ClientMessage:
            if (static_cast<Atom>(ev.xclient.data.l[0]) == impl_->wmDelete) closeRequested = true;
            break;
        case ConfigureNotify:
            impl_->width = ev.xconfigure.width;
            impl_->height = ev.xconfigure.height;
            break;
        case MotionNotify:
            impl_->mouseX = static_cast<float>(ev.xmotion.x);
            impl_->mouseY = static_cast<float>(ev.xmotion.y);
            break;
        case ButtonPress:
            impl_->mouseX = static_cast<float>(ev.xbutton.x);
            impl_->mouseY = static_cast<float>(ev.xbutton.y);
            if (ev.xbutton.button == Button1) {
                input.pressed = true;
                impl_->mouseDown = true;
            } else if (ev.xbutton.button == Button4) {
                input.wheel += 1;
            } else if (ev.xbutton.button == Button5) {
                input.wheel -= 1;
            }
            break;
        case ButtonRelease:
            impl_->mouseX = static_cast<float>(ev.xbutton.x);
            impl_->mouseY = static_cast<float>(ev.xbutton.y);
            if (ev.xbutton.button == Button1) {
                input.released = true;
                impl_->mouseDown = false;
            }
            break;
        case KeyPress: {
            char buf[16] = {0};
            KeySym sym = 0;
            const int n = XLookupString(&ev.xkey, buf, sizeof buf - 1, &sym, nullptr);
            switch (sym) {
            case XK_Return:
            case XK_KP_Enter:
                input.enter = true;
                break;
            case XK_Escape:
                input.escape = true;
                break;
            case XK_BackSpace:
                input.backspace = true;
                break;
            case XK_Tab:
                input.tab = true;
                break;
            default:
                for (int i = 0; i < n; ++i) {
                    const unsigned char c = static_cast<unsigned char>(buf[i]);
                    if (c >= 32 && c < 127) input.typed.push_back(static_cast<char>(c));
                }
                break;
            }
            break;
        }
        default:
            break;
        }
    }
    input.mouseX = impl_->mouseX;
    input.mouseY = impl_->mouseY;
    input.mouseDown = impl_->mouseDown;
}

void Window::Size(int &width, int &height) const {
    width = impl_->width;
    height = impl_->height;
}

void Window::Swap() {
    if (impl_->egl) impl_->egl->Swap();
}

jet::video::Display *Window::GlDisplay() const { return impl_->egl; }

unsigned Window::Framebuffer() const { return 0; }  // the EGL window surface is framebuffer 0

#elif defined(_WIN32)

struct Window::Impl {
    HWND hwnd = nullptr;
    jet::video::Win32Display *display = nullptr;
    int width = 0, height = 0;
    float mouseX = 0, mouseY = 0;
    bool mouseDown = false;
};

Window::Window() : impl_(new Impl) {}

Window::~Window() {
    Close();
    delete impl_;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window *self = nullptr;
    if (msg == WM_NCCREATE) {
        auto *cs = reinterpret_cast<CREATESTRUCTA *>(lParam);
        self = static_cast<Window *>(cs->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<Window *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    }
    if (!self) return DefWindowProcA(hwnd, msg, wParam, lParam);

    auto &impl = *self->impl_;
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        impl.width = static_cast<int>(LOWORD(lParam));
        impl.height = static_cast<int>(HIWORD(lParam));
        return 0;
    case WM_MOUSEMOVE:
        impl.mouseX = static_cast<float>(LOWORD(lParam));
        impl.mouseY = static_cast<float>(HIWORD(lParam));
        return 0;
    case WM_LBUTTONDOWN:
        impl.mouseDown = true;
        return 0;
    case WM_LBUTTONUP:
        impl.mouseDown = false;
        return 0;
    case WM_MOUSEWHEEL:
        impl.mouseX = static_cast<float>(LOWORD(lParam));
        impl.mouseY = static_cast<float>(HIWORD(lParam));
        return 0;
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

bool Window::Open(const std::string &title, int width, int height, std::string &error) {
    if (impl_->hwnd) {
        error = "the window is already open";
        return false;
    }

    HINSTANCE hinst = GetModuleHandleA(nullptr);

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hinst;
    wc.lpszClassName = "JetEditorWindowClass";
    if (!RegisterClassExA(&wc)) {
        error = "cannot register window class";
        return false;
    }

    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;

    HWND hwnd = CreateWindowExA(0, "JetEditorWindowClass", title.c_str(), style,
                                CW_USEDEFAULT, CW_USEDEFAULT, w, h,
                                nullptr, nullptr, hinst, this);
    if (!hwnd) {
        error = "cannot create window";
        return false;
    }

    impl_->hwnd = hwnd;
    impl_->width = width;
    impl_->height = height;

    impl_->display = new jet::video::Win32Display(hwnd);
    jet::video::DisplayMode mode;
    mode.size.x = static_cast<unsigned int>(width);
    mode.size.y = static_cast<unsigned int>(height);
    if (!impl_->display->Init(mode)) {
        error = "WGL setup failed for the editor window";
        Close();
        return false;
    }
    return true;
}

void Window::Close() {
    if (impl_->display) {
        delete impl_->display;
        impl_->display = nullptr;
    }
    if (impl_->hwnd) {
        DestroyWindow(impl_->hwnd);
        impl_->hwnd = nullptr;
    }
}

bool Window::IsOpen() const { return impl_->hwnd != nullptr && impl_->display != nullptr; }

void Window::Poll(ui::Input &input, bool &closeRequested, bool &anyEvent) {
    input.pressed = false;
    input.released = false;
    input.wheel = 0;
    input.typed.clear();
    input.enter = input.escape = input.backspace = input.tab = false;
    closeRequested = false;
    anyEvent = false;
    if (!IsOpen()) return;

    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            closeRequested = true;
            return;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
        anyEvent = true;
    }

    input.mouseX = impl_->mouseX;
    input.mouseY = impl_->mouseY;
    input.mouseDown = impl_->mouseDown;
}

void Window::Size(int &width, int &height) const {
    width = impl_->width;
    height = impl_->height;
}

void Window::Swap() {
    if (impl_->display) impl_->display->Swap();
}

jet::video::Display *Window::GlDisplay() const { return impl_->display; }

unsigned Window::Framebuffer() const { return 0; }

#else  // not Linux, not Windows (macOS fallback handled by window_mac.mm)

struct Window::Impl {};

Window::Window() : impl_(new Impl) {}
Window::~Window() { delete impl_; }

bool Window::Open(const std::string &, int, int, std::string &error) {
    error = "the editor window is not built on this platform yet (Linux has it). "
            "--selftest, --new and --import run here";
    return false;
}
void Window::Close() {}
bool Window::IsOpen() const { return false; }
void Window::Poll(ui::Input &input, bool &closeRequested, bool &anyEvent) {
    input = ui::Input{};
    closeRequested = false;
    anyEvent = false;
}
void Window::Size(int &width, int &height) const {
    width = 0;
    height = 0;
}
void Window::Swap() {}
jet::video::Display *Window::GlDisplay() const { return nullptr; }
unsigned Window::Framebuffer() const { return 0; }

#endif

}  // namespace jeteditor::app
