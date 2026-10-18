// tests/test_menu_window.mm
// The main-menu lab scene (test_menu) animated live in a Cocoa window:
//   test_menu_window <data dir> [width height]
#define JET_MENU_NO_MAIN
#import <Cocoa/Cocoa.h>
#include "test_menu.cpp"
#include "game/Game.h"

static NSWindow *g_window;
static NSImageView *g_view;

static bool Present(const unsigned char *px, unsigned w, unsigned h, void*) {
    static int n = 0;
    if (n++ < 2) { unsigned lit = 0; for (unsigned i = 0; i < w * h; i++) lit += px[i * 4] | px[i * 4 + 1] | px[i * 4 + 2] ? 1 : 0; printf("frame %d: %u lit pixels\n", n, lit); fflush(stdout); }
    @autoreleasepool {
        NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:w pixelsHigh:h
            bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:w * 4 bitsPerPixel:32];
        for (unsigned y = 0; y < h; y++) {
            unsigned char *d = [rep bitmapData] + y * w * 4;
            memcpy(d, px + (h - 1 - y) * w * 4, w * 4);
            for (unsigned x = 0; x < w; x++) d[x * 4 + 3] = 255;
        }
        NSImage *img = [[NSImage alloc] initWithSize:NSMakeSize(w, h)];
        [img addRepresentation:rep];
        [g_view setImage:img];
        [g_view display];
        // uncapped: the pending events are taken without waiting, the frame rate goes in the title twice a second
        static double lastTitle = 0;
        double nowS = [NSDate timeIntervalSinceReferenceDate];
        if (nowS - lastTitle > 0.5) {
            lastTitle = nowS;
            float fps = Game::GetInstance() ? Game::GetInstance()->GetFps().GetAverageFps() : 0.0f;
            [g_window setTitle:[NSString stringWithFormat:@"Minion Rush - main menu - %.0f fps", fps]];
        }
        for (;;) {
            NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
            if (!e) break;
            [NSApp sendEvent:e];
        }
        return [g_window isVisible];
    }
}

int main(int argc, char **argv) {
    @autoreleasepool {
        unsigned w = argc > 3 ? atoi(argv[2]) : 768, h = argc > 3 ? atoi(argv[3]) : 1024;
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        g_window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, w * 0.75, h * 0.75)
            styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];
        [g_window setTitle:@"Minion Rush - main menu"];
        g_view = [[NSImageView alloc] initWithFrame:[[g_window contentView] bounds]];
        [g_view setImageScaling:NSImageScaleProportionallyUpOrDown];
        [g_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [[g_window contentView] addSubview:g_view];
        [g_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp finishLaunching];
        char *args[] = { argv[0], argv[1], (char*)"x", (char*)"x", 0 };
        std::string ws = std::to_string(w), hs = std::to_string(h);
        args[2] = (char*)ws.c_str(); args[3] = (char*)hs.c_str();
        return MenuMain(4, args, Present, 0);
    }
}
