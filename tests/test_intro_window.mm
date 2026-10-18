// tests/test_intro_window.mm
// The game from launch (test_intro) running live in a Cocoa window:
//   test_intro_window <data dir> [width height]
#define JET_INTRO_NO_MAIN
#import <Cocoa/Cocoa.h>
#include "test_intro.cpp"

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
            [g_window setTitle:[NSString stringWithFormat:@"Minion Rush - %.0f fps", fps]];
        }
        for (;;) {
            NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
            if (!e) break;
            NSEventType t = [e type];
            if ((t == NSEventTypeLeftMouseDown || t == NSEventTypeLeftMouseDragged || t == NSEventTypeLeftMouseUp) && [e window] == g_window) {
                NSRect b = [g_view bounds];
                NSPoint p = [g_view convertPoint:[e locationInWindow] fromView:nil];
                double sc = std::min(b.size.width / w, b.size.height / h);
                double ox = (b.size.width - w * sc) / 2, oy = (b.size.height - h * sc) / 2;
                float x = (float)((p.x - ox) / sc), y = (float)(h - (p.y - oy) / sc);
                if (TouchMgr *tm = TouchMgr::GetInstance()) {
                    if (t == NSEventTypeLeftMouseDown) tm->OnTouchPress(0, x, y);
                    else if (t == NSEventTypeLeftMouseDragged) tm->OnTouchDrag(0, x, y);
                    else tm->OnTouchRelease(0, x, y);
                }
                continue;
            }
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
        [g_window setTitle:@"Minion Rush"];
        g_view = [[NSImageView alloc] initWithFrame:[[g_window contentView] bounds]];
        [g_view setImageScaling:NSImageScaleProportionallyUpOrDown];
        [g_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [[g_window contentView] addSubview:g_view];
        [g_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp finishLaunching];
        // the profile (bananas, costumes, purchases) persists between launches: the headless runs keep a fresh folder unless
        // JET_SAVEDIR says otherwise, so the window build sets its own
        if (!getenv("JET_SAVEDIR")) {
            NSString *dir = [NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES).firstObject stringByAppendingPathComponent:@"MinionRushRevival"];
            setenv("JET_SAVEDIR", dir.UTF8String, 1);
        }
        char *args[] = { argv[0], argv[1], (char*)"intro", (char*)"x", (char*)"x", 0 };
        std::string ws = std::to_string(w), hs = std::to_string(h);
        args[3] = (char*)ws.c_str(); args[4] = (char*)hs.c_str();
        args[1] = argv[1];
        return IntroMain(5, args, Present, 0);
    }
}
