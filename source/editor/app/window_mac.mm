// source/editor/app/window_mac.mm
//
// The editor's window on macOS: an NSWindow with an NSImageView in it. The engine's MacDisplay (core/jet, a CGL context
// with an offscreen GLES20RenderTarget, so there is no window surface to swap) is the GL display. Swap() reads the
// framebuffer back with glReadPixels and shows it in the view, the way tests/test_intro_window.mm does for the game.
//
// The project is built without ARC, so the objects alloc'd here are released by hand.
#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>

#include "editor/app/window.h"
#include "editor/ui/ui.h"
#include "video/displaymode.h"
#include "video/gles20/gles20rendertargetwrapper.h"
#include "video/mac/macdisplay.h"
#include "video/rendertarget.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace jeteditor::app {

struct Window::Impl {
    NSWindow *window = nil;
    NSImageView *view = nil;
    int width = 0, height = 0;
    float mouseX = 0, mouseY = 0;
    bool mouseDown = false;
    jet::video::MacDisplay *gl = nullptr;
};

Window::Window() : impl_(new Impl) {}

Window::~Window() {
    Close();
    delete impl_;
}

bool Window::Open(const std::string &title, int width, int height, std::string &error) {
    if (impl_->window) {
        error = "the window is already open";
        return false;
    }
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        NSWindow *window = [[NSWindow alloc]
            initWithContentRect:NSMakeRect(100, 100, width, height)
                      styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable |
                                NSWindowStyleMaskMiniaturizable
                        backing:NSBackingStoreBuffered
                          defer:NO];
        [window setReleasedWhenClosed:NO];
        [window setTitle:[NSString stringWithUTF8String:title.c_str()]];
        [window setAcceptsMouseMovedEvents:YES];
        NSImageView *view = [[NSImageView alloc] initWithFrame:[[window contentView] bounds]];
        [view setImageScaling:NSImageScaleProportionallyUpOrDown];
        [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [[window contentView] addSubview:view];
        [view release];  // the content view keeps it
        [window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp finishLaunching];

        jet::video::DisplayMode mode;
        mode.size.x = static_cast<unsigned int>(width);
        mode.size.y = static_cast<unsigned int>(height);
        jet::video::MacDisplay *gl = new jet::video::MacDisplay();
        if (!gl->Init(mode)) {
            delete gl;
            [window close];
            [window release];
            error = "GL setup failed for the editor window (no CGL context)";
            return false;
        }
        impl_->window = window;
        impl_->view = view;
        impl_->gl = gl;
        impl_->width = width;
        impl_->height = height;
    }
    return true;
}

void Window::Close() {
    if (impl_->gl) {
        delete impl_->gl;
        impl_->gl = nullptr;
    }
    if (impl_->window) {
        [impl_->window close];
        [impl_->window release];
        impl_->window = nil;
        impl_->view = nil;
    }
}

bool Window::IsOpen() const { return impl_->window != nil && impl_->gl != nullptr; }

void Window::Poll(ui::Input &input, bool &closeRequested, bool &anyEvent) {
    input.pressed = false;
    input.released = false;
    input.wheel = 0;
    input.typed.clear();
    input.enter = input.escape = input.backspace = input.tab = false;
    closeRequested = false;
    anyEvent = false;
    if (!IsOpen()) return;

    // The size of the image is the content view's, so the frame and the mouse are in the same pixels
    const NSRect bounds = [impl_->view bounds];
    impl_->width = static_cast<int>(bounds.size.width);
    impl_->height = static_cast<int>(bounds.size.height);
    // The frame is drawn into the display's render target, which is made the window's size (a minimised window is 0)
    if (impl_->width > 0 && impl_->height > 0)
        impl_->gl->Resize(math::vec2<unsigned int>(static_cast<unsigned int>(impl_->width), static_cast<unsigned int>(impl_->height)));

    // Mouse positions come in the view's coordinates, which start at the bottom: the UI's start at the top
    auto locate = [&](NSEvent *e) {
        const NSPoint p = [impl_->view convertPoint:[e locationInWindow] fromView:nil];
        impl_->mouseX = static_cast<float>(p.x);
        impl_->mouseY = static_cast<float>(impl_->height - p.y);
    };

    @autoreleasepool {
        for (;;) {
            NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:[NSDate distantPast]
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES];
            if (!e) break;
            anyEvent = true;
            const NSEventType type = [e type];
            switch (type) {
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
                locate(e);
                break;
            case NSEventTypeLeftMouseDown:
                locate(e);
                input.pressed = true;
                impl_->mouseDown = true;
                break;
            case NSEventTypeLeftMouseUp:
                locate(e);
                input.released = true;
                impl_->mouseDown = false;
                break;
            case NSEventTypeScrollWheel: {
                const CGFloat dy = [e scrollingDeltaY];
                if (dy > 0) input.wheel += 1;
                else if (dy < 0) input.wheel -= 1;
                break;
            }
            case NSEventTypeKeyDown: {
                switch ([e keyCode]) {
                case 36:  // Return
                case 76:  // keypad Enter
                    input.enter = true;
                    break;
                case 53:  // Escape
                    input.escape = true;
                    break;
                case 51:  // Delete, the key that is backspace on the Mac
                    input.backspace = true;
                    break;
                case 48:  // Tab
                    input.tab = true;
                    break;
                default: {
                    NSString *chars = [e characters];
                    for (NSUInteger i = 0; i < [chars length]; ++i) {
                        const unichar c = [chars characterAtIndex:i];
                        if (c >= 32 && c < 127) input.typed.push_back(static_cast<char>(c));
                    }
                    break;
                }
                }
                break;
            }
            default:
                break;
            }
            // The keys and the wheel are the editor's; everything else goes to the application as usual
            if (type != NSEventTypeKeyDown && type != NSEventTypeScrollWheel) [NSApp sendEvent:e];
        }
    }

    input.mouseX = impl_->mouseX;
    input.mouseY = impl_->mouseY;
    input.mouseDown = impl_->mouseDown;
    // The close button takes the window off the screen; a minimised window is not visible either, and is not a close
    closeRequested = ![impl_->window isVisible] && ![impl_->window isMiniaturized];
}

void Window::Size(int &width, int &height) const {
    width = impl_->width;
    height = impl_->height;
}

void Window::Swap() {
    if (!IsOpen()) return;
    const int w = impl_->width, h = impl_->height;
    if (w < 1 || h < 1) return;
    const size_t rowBytes = static_cast<size_t>(w) * 4;
    // glReadPixels reads the framebuffer the editor drew into, bottom row first; the flush comes with the read
    std::vector<uint8_t> raw(rowBytes * static_cast<size_t>(h));
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer());
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, raw.data());
    @autoreleasepool {
        NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:w pixelsHigh:h
            bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace
            bytesPerRow:w * 4 bitsPerPixel:32];
        for (int y = 0; y < h; y++) {
            uint8_t *d = [rep bitmapData] + static_cast<size_t>(y) * rowBytes;
            memcpy(d, raw.data() + static_cast<size_t>(h - 1 - y) * rowBytes, rowBytes);
            for (int x = 0; x < w; x++) d[x * 4 + 3] = 255;
        }
        NSImage *img = [[NSImage alloc] initWithSize:NSMakeSize(w, h)];
        [img addRepresentation:rep];
        [impl_->view setImage:img];
        [impl_->view display];
        [img release];  // the view keeps the image, the image keeps the representation
        [rep release];
    }
}

jet::video::Display *Window::GlDisplay() const { return impl_->gl; }

unsigned Window::Framebuffer() const {
    if (!impl_->gl) return 0;
    jet::video::GLES20RenderTargetWrapper *target =
        dynamic_cast<jet::video::GLES20RenderTargetWrapper *>(impl_->gl->GetRenderTarget().get());
    return target ? target->GetFBOId() : 0;
}

}  // namespace jeteditor::app
