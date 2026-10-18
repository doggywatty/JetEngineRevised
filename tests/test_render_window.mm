// tests/test_render_window.mm
// Visible on-screen render test: opens a real Cocoa window with an NSOpenGLView, wires the
// GLES20 driver stack to its default framebuffer (fboId 0, via GLES20RenderTargetWrapper -- the
// on-screen case that class documents itself as supporting), and redraws the same real
// Painter-drawn triangle every frame via an NSTimer, so a person can watch it render live.
#import <Cocoa/Cocoa.h>
#define GL_SILENCE_DEPRECATION
#include "video/gles20/gles20driver.h"
#include "video/gles20/gles20rendertargetwrapper.h"
#include "video/painter.h"
#include "video/rendertechnique.h"
#include "video/material.h"
#include "stream/memorystream.h"

using namespace jet;
using namespace jet::video;

static const char *kVS =
    "attribute vec3 a_position;\n"
    "attribute vec4 a_color;\n"
    "varying vec4 v_color;\n"
    "void main() { v_color = a_color; gl_Position = vec4(a_position, 1.0); }\n";
static const char *kFS =
    "#ifdef GL_ES\nprecision mediump float;\n#endif\n"
    "varying vec4 v_color;\n"
    "void main() { gl_FragColor = v_color; }\n";
static const char *kTechniqueXml =
    "<technique version=\"100\">"
    "<renderpasses><pass name=\"main\">"
    "<vertexshader><![CDATA[__VS__]]></vertexshader>"
    "<fragmentshader><![CDATA[__FS__]]></fragmentshader>"
    "<attribute name=\"mesh.positions\"/>"
    "<attribute name=\"mesh.colors\"/>"
    "</pass></renderpasses>"
    "</technique>";

@interface JetGLView : NSOpenGLView
@property(nonatomic) GLES20Driver *driver;
@property(nonatomic) boost::shared_ptr<RenderTarget> *rt;
@property(nonatomic) boost::shared_ptr<Material> *mat;
@end

@implementation JetGLView
- (void)prepareOpenGL {
    [super prepareOpenGL];
    [[self openGLContext] makeCurrentContext];

    self.driver = new GLES20Driver();
    self.driver->Init(nullptr);

    self.rt = new boost::shared_ptr<RenderTarget>(new GLES20RenderTargetWrapper(0));
    NSSize sz = [self convertSizeToBacking:self.bounds.size];
    (*self.rt)->Create("window", "rgba8888", math::vec2<unsigned int>((unsigned)sz.width, (unsigned)sz.height));

    std::string xml(kTechniqueXml);
    size_t p = xml.find("__VS__"); xml.replace(p, 6, kVS);
    p = xml.find("__FS__"); xml.replace(p, 6, kFS);
    stream::MemoryStream *ms = new stream::MemoryStream();
    ms->Write(xml.data(), (unsigned)xml.size());
    ms->SeekBeg(0);
    boost::shared_ptr<RenderTechnique> tech(new RenderTechnique(String("win")));
    tech->Init(ms);
    tech->Load();
    tech->Link();

    self.mat = new boost::shared_ptr<Material>(new Material());
    (*self.mat)->SetRenderTechnique(tech);
}

- (void)drawRect:(NSRect)dirtyRect {
    [[self openGLContext] makeCurrentContext];
    self.driver->BeginRendering();
    self.driver->SetCrtRenderTarget(*self.rt);

    static float t = 0.0f;
    t += 0.02f;
    Painter painter(1024);
    painter.SetMaterial(**self.mat);
    Painter::Vertex a(math::vec3<float>(-0.8f, -0.8f, 0.0f), Color(1, 0, 0, 1));
    Painter::Vertex b(math::vec3<float>(0.8f, -0.8f, 0.0f), Color(0, 1, 0, 1));
    Painter::Vertex c(math::vec3<float>(std::sin(t) * 0.8f, 0.8f, 0.0f), Color(0, 0, 1, 1));
    painter.FillTriangle(a, b, c);
    painter.Flush();

    self.driver->EndRendering();
    [[self openGLContext] flushBuffer];
}
@end

int main() {
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSRect frame = NSMakeRect(200, 200, 512, 512);
        NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                        styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable)
                                                          backing:NSBackingStoreBuffered
                                                            defer:NO];
        [window setTitle:@"JetEngine render test"];

        NSOpenGLPixelFormatAttribute attrs[] = {
            NSOpenGLPFAColorSize, 32, NSOpenGLPFADepthSize, 24, NSOpenGLPFADoubleBuffer, 0
        };
        NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        JetGLView *view = [[JetGLView alloc] initWithFrame:frame pixelFormat:pf];
        [window setContentView:view];
        [window makeKeyAndOrderFront:nil];
        [app activateIgnoringOtherApps:YES];

        [NSTimer scheduledTimerWithTimeInterval:1.0/30.0 repeats:YES block:^(NSTimer *timer) {
            [view setNeedsDisplay:YES];
        }];

        [app run];
    }
    return 0;
}
