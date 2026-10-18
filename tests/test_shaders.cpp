#undef NDEBUG
// Loads and links every technique (*.xml) of the game's shaders directory on a real GLES2 context.
#include "video/rendertechnique.h"
#include "video/renderpass.h"
#include "video/shaderprogram.h"
#include "video/material.h"
#include "video/shaderprogram.h"
#include "video/shaderprogramflavor.h"
#include "video/renderpass.h"
#include "video/gles20/gles20driver.h"
#include "stream/streammgr.h"
#include "stream/filestream.h"
#if defined(__APPLE__)
#include "video/mac/macdisplay.h"
typedef jet::video::MacDisplay TestDisplay;
#else
#include "testdisplay.h"
#endif
#include <cassert>
#include <cstdio>
#include <dirent.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace jet;

int main(int argc, char **argv) {
    std::string dir = argc > 1 ? argv[1] : "shaders";
    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(16, 16);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    stream::StreamMgr::GetInstance().AddPath(String(dir.c_str()), String());

    std::vector<std::string> files;
    if (DIR *d = opendir(dir.c_str())) {
        while (dirent *e = readdir(d)) {
            std::string n = e->d_name;
            if (n.size() > 4 && n.substr(n.size() - 4) == ".xml" && (argc < 3 || n == argv[2])) files.push_back(n);
        }
        closedir(d);
    }
    std::sort(files.begin(), files.end());
    unsigned ok = 0, bad = 0, skinBad = 0;
    for (const std::string &f : files) {
        stream::FileStream *s = new stream::FileStream(String((dir + "/" + f).c_str()), stream::FileStream::k_in, false);
        boost::shared_ptr<video::RenderTechnique> tech(new video::RenderTechnique(String(f.c_str())));
        tech->Init(s);
        bool loaded = tech->Load();
        bool linked = false;
        if (loaded) {
            tech->Link();
            linked = true;
            for (unsigned p = 0; p < tech->GetRenderPassCount(); p++)
                if (!tech->GetRenderPass(p).GetShaderProgram()) linked = false;
            video::Material m;
            m.SetRenderTechnique(tech);
            // the plain flavor and a hardware skinned one (2 node weights) must both compile
            typedef video::ShaderProgramFlavor::TypeDef FT;
            for (unsigned p = 0; linked && p < tech->GetRenderPassCount(); p++) {
                video::ShaderProgram *prog = tech->GetRenderPass(p).GetShaderProgram();
                if (!prog->GetFlavor(safe_enum<FT,FT::type>(FT::Normal), m, p, std::vector<video::ShaderUniform>())) linked = false;
                if (tech->GetRenderPass(p).GetType() != video::RenderPass::TypeDef::Quad &&
                    !prog->GetFlavor(safe_enum<FT,FT::type>(FT::Skinned2), m, p, std::vector<video::ShaderUniform>())) {
                    printf("  skinned flavor failed\n");
                    ++skinBad;
                }
            }
        }
        printf("%-50s load %d link %d passes %u\n", f.c_str(), loaded, linked, loaded ? tech->GetRenderPassCount() : 0);
        (loaded && linked ? ok : bad)++;
    }
    printf("techniques ok %u bad %u (skinned flavor failures %u)\n", ok, bad, skinBad);
    display.Cleanup();
}
