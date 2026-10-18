// tests/test_aurora.cpp
#undef NDEBUG
// Aurora GUI layers: gui_1_1 loaded with its element names, the MainMenu layer printed after being
// scaled to the display:
//   test_aurora <data dir> [layer] [width height]
#include "testdisplay.h"
#include "video/gles20/gles20driver.h"
#include "stream/streammgr.h"
#include "stream/libstreamfactory.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace jet;

int main(int argc, char **argv) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string layer = argc > 2 ? argv[2] : "MainMenu";
    const unsigned W = argc > 4 ? atoi(argv[3]) : 768, H = argc > 4 ? atoi(argv[4]) : 1024;
    if (FILE *f = fopen((data + "/datalibs/gui_1_1").c_str(), "rb")) fclose(f);
    else { printf("no gui_1_1 under %s, skipped\n", data.c_str()); return 0; }
    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(W, H);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    sm.AddPath(String((data + "/datalibs").c_str()), String());

    boost::shared_ptr<stream::IStreamFactory> layers(new stream::LibStreamFactory(String((data + "/datalibs/gui_1_1").c_str()), String()));
    AuroraLevelMgr mgr;
    mgr.Load(layers, String("gui_1_1_str"), true, 768, 1024, 1.0f, std::vector<String>());
    printf("%u levels\n", mgr.GetLevelCount());
    AuroraLevelData *l = mgr.GetLevel(String(layer.c_str()));
    assert(l);
    printf("%s: %u elements\n", l->GetLevelName().c_str(), l->GetElementsCount());
    static const char *types[] = { "Graph", "Area", "FillRect", "Button", "Text", "List", "ListButtonScroll", "Slide", "PBar", "Grid",
                                   "DeviceSpecificAnim", "ScrollBar", "3DElement", "3DBodyPart", "3DCostumePos", "3DTouchInteract" };
    for (unsigned i = 0; i < l->GetElementsCount(); i++) {
        int t = l->GetParamValue(i, 0);
        int wid = l->GetElementWidthParameterId(i);
        printf("%3u %-18s %-28s id=%-3d pos=(%d,%d)", i, t < 16 ? types[t] : "?", l->GetElementName(i).c_str(), l->GetParamValue(i, 1),
               l->GetParamValue(i, 2), l->GetParamValue(i, 3));
        if (wid > 0) printf(" size=(%d,%d)", l->GetParamValue(i, wid), l->GetParamValue(i, wid + 1));
        printf("\n");
    }
    display.Cleanup();
    return 0;
}
