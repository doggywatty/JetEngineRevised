// tests/test_frame.cpp
#undef NDEBUG
// Renders one frame of a game model through the engine (ModelBase, the game's technique XML,
// PVR textures, the GLES2 driver) offscreen and writes it as a PPM:
//   test_frame <data dir> [model.pig] [out.ppm]
// The model's materials are set up the way MaterialDef/RenderPassDef/GameEntity::ApplyMaterials
// do for MAT_Labo_concrete / MAT_glass (designlib), which lablib's DEC_labo_minion_room uses.
#include "testdisplay.h"
#include "video/gles20/gles20driver.h"
#include "video/material.h"
#include "video/textureloader.h"
#include "video/rendertarget.h"
#include "scene/modelbase.h"
#include "scene/mesh.h"
#include "scene/submesh.h"
#include "scene/model.h"
#include "scene/camera.h"
#include "stream/streammgr.h"
#include "stream/filestream.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace jet;

struct PassDef {
    const char *shader;
    std::vector<float> values;
    std::vector<const char*> textures;
    const char *blending;
};

// Placeholder texture names of RenderPassDef::LoadMaterialTextures, by semantic index
static int PlaceholderSemantic(const std::string &t) {
    static const char *names[] = { "<Diffuse>", "<Ambient>", "<Bump>", "<SpecularCol>", "<SpecularLvl>", "<Glossiness>",
                                   "<SelfIllum>", "<Opacity>", "<FilterCol>", "<Reflection>", "<Refraction>", "<Displacement>" };
    for (int i = 0; i < 12; i++) if (t == names[i]) return i;
    return -1;
}

static void Apply(video::Material &mat, const PassDef &d, const std::vector<String> &modelTextures) {
    mat.SetRenderTechnique(String(d.shader));
    assert(mat.GetRenderTechnique());
    const unsigned pass = 0;
    // LoadMaterialValues: the material uniforms in order
    for (unsigned i = 0; i < d.values.size() && i < mat.GetUniformCount(pass); i++)
        mat.SetUniform(pass, i, video::ShaderUniform(d.values[i]));
    video::RenderState &rs = mat.GetRenderState(pass);
    std::string b = d.blending;
    using F = video::BlendFormula::FactorDef;
    F::type src = F::One, dst = F::Zero;
    if (b == "Additive") { src = F::One; dst = F::One; }
    else if (b == "Transparent") { src = F::SrcAlpha; dst = F::InvSrcAlpha; }
    else if (b == "Multiply") { src = F::Zero; dst = F::SrcColor; }
    else if (b == "ColorBurn") { src = F::DstColor; dst = F::One; }
    else if (b == "Screen") { src = F::One; dst = F::InvSrcColor; }
    else if (b == "Substractive") { src = F::Zero; dst = F::InvSrcColor; }
    rs.SetBlending(b != "Solid");
    rs.SetBlendFormula(video::BlendFormula(src, dst, src, dst));
    if (b != "Solid") rs.SetDepthWrite(false);
    // LoadMaterialTextures + ApplyMaterials: placeholders take the model's map slot
    video::TextureLoader *tl = video::TextureLoader::GetInstance();
    for (unsigned i = 0; i < d.textures.size() && i < mat.GetTextureLayerCount(pass); i++) {
        std::string t = d.textures[i];
        int sem = PlaceholderSemantic(t);
        if (sem >= 0) t = sem < (int)modelTextures.size() ? modelTextures[sem].c_str() : "";
        if (t.empty()) continue;
        video::TextureLayer &layer = mat.GetTextureLayer(pass, i);
        layer.SetTexture(tl->Load(String(t.c_str())));
        if (!layer.GetTexture()) printf("  missing texture %s\n", t.c_str());
        if (video::Driver::GetCurrent()->GetSeparateAlphaTextures())
            layer.SetAlphaTexture(tl->Load(String((t + "__alpha").c_str())));
    }
}

int main(int argc, char **argv) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string model = argc > 2 ? argv[2] : "labo_minion_room.pig";
    std::string out = argc > 3 ? argv[3] : "frame.ppm";
    const unsigned W = 960, H = 540;

    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(W, H);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    sm.AddPath(String((data + "/models").c_str()), String());
    sm.AddPath(String((data + "/shaders").c_str()), String());
    sm.AddPath(String((data + "/textures_android").c_str()), String());

    stream::IStream *s = sm.CreateStream(String(model.c_str()));
    if (!s) { printf("%s not found under %s, skipped\n", model.c_str(), data.c_str()); display.Cleanup(); return 0; }
    boost::shared_ptr<scene::ModelBase> base(new scene::ModelBase());
    base->SetResourceName(String(model.c_str()));
    base->Init(s);
    assert(base->Load());
    scene::Model *inst = base->CreateInstance();

    // MAT_Labo_concrete and MAT_glass (designlib.blibclara)
    PassDef concrete = { "staticlm_ndis.xml", { 0.4f, 0.3f, 0.7f, 0.7f },
                         { "<Diffuse>", "tile_metal_wall_grey_nm.tga", "tile_metal_wall_grey_mk.tga", "labo_lm.tga", "fx_ndis_reflection_q3.tga" }, "Solid" };
    PassDef glass = { "staticlm_ndis.xml", { 1.5f, 0.0f, 1.1f, 0.5f },
                      { "tile_glass.tga", "aux_norm_nonormals_nm.tga", "aux_col_red.tga", "aux_col_white.tga", "fx_ndis_reflection_q3.tga" }, "Transparent" };
    math::vec3<float> lo(1e9f, 1e9f, 1e9f), hi(-1e9f, -1e9f, -1e9f);
    for (unsigned i = 0; i < inst->GetMaterialCount(); i++) {
        const String &n = inst->GetMaterialName(i);
        printf("material %s\n", n.c_str());
        Apply(inst->GetMutableMaterial(i), n.Equals("glass") ? glass : concrete, base->GetMaterialData(i).textureNames);
    }
    for (unsigned m = 0; m < base->GetMultiResMeshCount(); m++)
        for (unsigned l = 0; l < scene::ModelBase::k_maxLodCount; l++) {
            boost::shared_ptr<scene::Mesh> mesh = base->GetMultiResMesh(m).lods[l];
            if (!mesh) continue;
            const math::aabb<float> &bb = mesh->GetBoundingBox();
            const math::mat4<float> &nt = inst->GetNode(base->GetMultiResMesh(m).nodeIdx).GetAbsoluteTransform();
            for (int k = 0; k < 8; k++) {
                math::vec3<float> p(k & 1 ? bb.maxPoint.x : bb.minPoint.x, k & 2 ? bb.maxPoint.y : bb.minPoint.y, k & 4 ? bb.maxPoint.z : bb.minPoint.z);
                p = math::transform(nt, p);
                lo.x = std::min(lo.x, p.x); lo.y = std::min(lo.y, p.y); lo.z = std::min(lo.z, p.z);
                hi.x = std::max(hi.x, p.x); hi.y = std::max(hi.y, p.y); hi.z = std::max(hi.z, p.z);
            }
        }
    printf("bounds (%g %g %g) - (%g %g %g)\n", lo.x, lo.y, lo.z, hi.x, hi.y, hi.z);

    // Camera looking at the model from its -Y side, slightly above (jet space: Z up, front -Y
    // at identity rotation, so yaw pi turns it to look along +Y)
    math::vec3<float> c((lo.x + hi.x) * 0.5f, (lo.y + hi.y) * 0.5f, (lo.z + hi.z) * 0.5f);
    float radius = std::sqrt((hi.x - lo.x) * (hi.x - lo.x) + (hi.y - lo.y) * (hi.y - lo.y) + (hi.z - lo.z) * (hi.z - lo.z)) * 0.5f;
    float yaw = argc > 4 ? (float)atof(argv[4]) * 0.01745329f : 3.14159265f + 0.6f;
    float pitch = -0.35f;
    math::quat<float> qz(0, 0, std::sin(yaw * 0.5f), std::cos(yaw * 0.5f));
    math::quat<float> qx(std::sin(pitch * 0.5f), 0, 0, std::cos(pitch * 0.5f));
    scene::Camera cam(math::vec3<float>(0, 0, 0), qz * qx);
    cam.SetViewport(math::rect<unsigned int>(0, 0, W, H));
    cam.SetFov(50.0f * 0.01745329f);
    cam.SetNearDistance(0.1f);
    cam.SetFarDistance(radius * 10.0f);
    math::vec3<float> front = cam.GetFrontVector();
    float dist = radius * 1.6f;
    cam.SetPosition(math::vec3<float>(c.x - front.x * dist, c.y - front.y * dist, c.z - front.z * dist));
    printf("camera pos (%g %g %g) front (%g %g %g)\n", cam.GetPosition().x, cam.GetPosition().y, cam.GetPosition().z,
           cam.GetFrontVector().x, cam.GetFrontVector().y, cam.GetFrontVector().z);

    if (getenv("JET_TRACE")) {
        const math::mat4<float> vp = cam.GetProjectionTransform() * cam.GetViewTransform();
        boost::shared_ptr<scene::Mesh> mesh = base->GetMultiResMesh(0).lods[1] ? base->GetMultiResMesh(0).lods[1] : base->GetMultiResMesh(0).lods[0];
        for (unsigned i = 0; mesh && i < mesh->GetSubMeshCount(); i++) {
            const scene::SubMesh &sub = mesh->GetSubMesh(i);
            boost::shared_ptr<video::Geometry> g = sub.GetGeometry();
            int a = g->FindAttributeIdxBySemantic(video::Geometry::SemanticDef::Position);
            const unsigned char *d = g->MapAttributeU8Read(a);
            unsigned stride = g->GetAttributeStride(a);
            printf("submesh %u q=%u type=%u stride %u off (%g %g %g) scale (%g %g %g)\n", i, sub.GetPositionsQuantization(), 0u, stride,
                   sub.GetPositionsQuantizationOffset().x, sub.GetPositionsQuantizationOffset().y, sub.GetPositionsQuantizationOffset().z,
                   sub.GetPositionsQuantizationScale().x, sub.GetPositionsQuantizationScale().y, sub.GetPositionsQuantizationScale().z);
            for (unsigned v = 0; v < 3 && v < g->GetVertexCount(); v++) {
                math::vec3<float> p;
                if (sub.GetPositionsQuantization() == 1) {
                    const short *q = (const short*)(d + v * stride);
                    p = math::vec3<float>(q[0] / 32767.f, q[1] / 32767.f, q[2] / 32767.f);
                    p = math::vec3<float>(sub.GetPositionsQuantizationOffset().x + p.x * sub.GetPositionsQuantizationScale().x,
                                          sub.GetPositionsQuantizationOffset().y + p.y * sub.GetPositionsQuantizationScale().y,
                                          sub.GetPositionsQuantizationOffset().z + p.z * sub.GetPositionsQuantizationScale().z);
                } else memcpy(&p, d + v * stride, 12);
                math::vec4<float> c = math::transform(vp, math::vec4<float>(p, 1.0f));
                printf("  v%u (%g %g %g) ndc (%g %g %g)\n", v, p.x, p.y, p.z, c.x / c.w, c.y / c.w, c.z / c.w);
            }
            g->UnmapAttribute(a);
        }
    }
    driver.BeginRendering();
    driver.SetCrtRenderTarget(display.GetRenderTarget());
    glViewport(0, 0, W, H);
    glClearColor(0.1f, 0.1f, 0.12f, 1);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    driver.SetCamera(&cam);
    driver.AddTime(16);
    inst->Render();
    if (getenv("JET_TRACE")) {
        GLint prog = 0, n = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &n);
        for (GLint i = 0; i < n; i++) {
            char name[128]; GLint size; GLenum type;
            glGetActiveUniform(prog, i, sizeof(name), 0, &size, &type, name);
            GLint loc = glGetUniformLocation(prog, name);
            float v[16] = { 0 };
            if (type != GL_SAMPLER_2D && type != GL_INT) glGetUniformfv(prog, loc, v);
            printf("uniform %s type 0x%x: %g %g %g %g | %g %g %g %g ...\n", name, type, v[0], v[1], v[2], v[3], v[12], v[13], v[14], v[15]);
        }
        GLboolean cm[4]; glGetBooleanv(GL_COLOR_WRITEMASK, cm);
        GLint df, cf, vp[4]; glGetIntegerv(GL_DEPTH_FUNC, &df); glGetIntegerv(GL_CULL_FACE_MODE, &cf); glGetIntegerv(GL_VIEWPORT, vp);
        GLfloat dr[2]; glGetFloatv(GL_DEPTH_RANGE, dr);
        printf("colormask %d%d%d%d depthtest %d func 0x%x cull %d face 0x%x blend %d stencil %d scissor %d viewport %d %d %d %d range %g %g\n", cm[0], cm[1], cm[2], cm[3],
               glIsEnabled(GL_DEPTH_TEST), df, glIsEnabled(GL_CULL_FACE), cf, glIsEnabled(GL_BLEND), glIsEnabled(GL_STENCIL_TEST), glIsEnabled(GL_SCISSOR_TEST), vp[0], vp[1], vp[2], vp[3], dr[0], dr[1]);
        glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &n);
        for (GLint i = 0; i < n; i++) {
            char name[128]; GLint size; GLenum type;
            glGetActiveAttrib(prog, i, sizeof(name), 0, &size, &type, name);
            GLint loc = glGetAttribLocation(prog, name), en = 0;
            glGetVertexAttribiv(loc, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &en);
            printf("attrib %s loc %d enabled %d\n", name, loc, en);
        }
    }
    driver.EndRendering();
    printf("glGetError 0x%x\n", glGetError());

    std::vector<unsigned char> px(W * H * 4);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    FILE *f = fopen(out.c_str(), "wb");
    fprintf(f, "P6\n%u %u\n255\n", W, H);
    unsigned lit = 0;
    for (int y = (int)H - 1; y >= 0; y--)
        for (unsigned x = 0; x < W; x++) {
            const unsigned char *p = &px[(y * W + x) * 4];
            fwrite(p, 1, 3, f);
            if (p[0] > 40 || p[1] > 40 || p[2] > 40) ++lit;
        }
    fclose(f);
    printf("wrote %s, %u lit pixels\n", out.c_str(), lit);
    delete inst;
    display.Cleanup();
    return 0;
}
