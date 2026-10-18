// source/game/utils/GameUtils.cpp
#include "GameUtils.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "scene/model.h"
#include "scene/scenemgr.h"
#include "scene/camera.h"
#include "video/driver.h"
#include "video/rendertarget.h"
#include "game/GameLevel.h"
#include "game/utils/ControlledRandom.h"
#include "core/util.h"
#include "game/gamemanagers/PainterMgr.h"
#include "video/material.h"

// @0x3bc7c8: the painter's material with the unlit "_primitive2" technique, alpha blended, no culling or depth
void GameUtils::FillRect(jet::video::Painter &painter, const math::rect<float> &r, const jet::video::Color &color, const char *technique) {
    typedef jet::video::Painter::Vertex Vertex;
    jet::video::Material saved(painter.GetMaterial());
    jet::video::Material m(saved);
    m.SetColor(0xffffffff);
    m.SetRenderTechnique(jet::String(technique));
    jet::video::RenderState &rs = m.GetRenderState(0);
    rs.SetBlending(true);
    rs.SetBlendFormula(jet::video::BlendFormula(jet::video::BlendFormula::ComboDef::Alpha));
    rs.SetCulling(false);
    rs.SetDepthTest(false);
    rs.SetDepthWrite(false);
    painter.SetMaterial(m);
    float x0 = r.upperLeftCorner.x, y0 = r.upperLeftCorner.y, x1 = r.lowerRightCorner.x, y1 = r.lowerRightCorner.y;
    Vertex a(math::vec3<float>(x0, y1, 0), color, math::vec2<float>(0, 0));
    Vertex b(math::vec3<float>(x1, y1, 0), color, math::vec2<float>(1, 0));
    Vertex c(math::vec3<float>(x1, y0, 0), color, math::vec2<float>(1, 1));
    Vertex d(math::vec3<float>(x0, y0, 0), color, math::vec2<float>(0, 1));
    painter.FillQuad(a, b, c, d);
    painter.SetMaterial(saved);
}

void GameUtils::DrawRect(jet::video::Painter &p, const math::rect<float> &r, const jet::video::Color &c) { FillRect(p, r, c, "_primitive2"); }
void GameUtils::DrawRectWorld(jet::video::Painter &p, const math::rect<float> &r, const jet::video::Color &c) { FillRect(p, r, c, "_primitive2world"); }

void GameUtils::GetProjectedAABB(const jet::scene::Camera &camera, const math::aabb<float> &box, math::rect<float> &out) {
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(camera);
    const math::vec3<float> &a = box.minPoint, &b = box.maxPoint;
    math::vec3<float> corners[8] = { math::vec3<float>(a.x, a.y, a.z), math::vec3<float>(b.x, a.y, a.z), math::vec3<float>(a.x, b.y, a.z), math::vec3<float>(b.x, b.y, a.z),
                                     math::vec3<float>(a.x, a.y, b.z), math::vec3<float>(b.x, a.y, b.z), math::vec3<float>(a.x, b.y, b.z), math::vec3<float>(b.x, b.y, b.z) };
    math::vec3<float> p(0, 0, 0);
    cam.Project(p, corners[0]);
    float x0 = p.x, y0 = p.y, x1 = p.x, y1 = p.y;
    for (int i = 1; i < 8; i++) {
        cam.Project(p, corners[i]);
        if (p.x > x1) x1 = p.x;
        if (p.y > y1) y1 = p.y;
        if (p.x < x0) x0 = p.x;
        if (p.y < y0) y0 = p.y;
    }
    x0 -= 50.0f;
    y1 += 50.0f;
    if (x1 < x0) { x1 = x0; x0 = x1; }
    if (y1 < y0) { y1 = y0; y0 = y1; }
    out = math::rect<float>(x0, y0, x1, y1);
}

// @0x3bd43c: a depth-only quad over the rectangle that always passes the depth test
void GameUtils::ClearDepthBuffer(const math::rect<float> &r) {
    typedef jet::video::Painter::Vertex Vertex;
    if (!(r.lowerRightCorner.x - r.upperLeftCorner.x > 0.0f) || !(r.lowerRightCorner.y - r.upperLeftCorner.y > 0.0f)) return;
    jet::video::Painter &painter = PainterMgr::GetInstance()->Get2DPainter();
    painter.Flush();
    jet::video::Material saved(painter.GetMaterial());
    jet::video::Material m;
    m.SetRenderTechnique(jet::String("_clear_rt_depth_screen_coords.xml"));
    jet::video::RenderState &rs = m.GetRenderState(0);
    rs.SetDepthFunc(jet::video::RenderState::CompareFuncDef::Always);
    rs.SetColorMask(false, false, false, false);
    painter.SetMaterial(m);
    jet::video::Color white(1, 1, 1, 1);
    float x0 = r.upperLeftCorner.x, y0 = r.upperLeftCorner.y, x1 = r.lowerRightCorner.x, y1 = r.lowerRightCorner.y;
    Vertex a(math::vec3<float>(x0, y0, 0), white, math::vec2<float>(0, 0));
    Vertex b(math::vec3<float>(x1, y1, 0), white, math::vec2<float>(0, 0));
    painter.FillRectangle(a, b);
    painter.Flush();
    painter.SetMaterial(saved);
}

void GameUtils::ClearDepthBuffer() {
    const math::vec2<unsigned int> &s = jet::video::Driver::GetCurrent()->GetCrtRenderTarget()->GetSize();
    ClearDepthBuffer(math::rect<float>(0, 0, (float)s.x, (float)s.y));
}

// @0x3bd1bc: the union of the world boxes of the models the tag will draw, projected
void GameUtils::ClearDepthBuffer(jet::scene::SceneMgr *sm, unsigned tag) {
    math::aabb<float> box(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1));
    bool any = false;
    for (unsigned i = 0, n = sm->GetRenderableCount(tag); i < n; i++) {
        jet::scene::Model *m = dynamic_cast<jet::scene::Model*>(sm->GetRenderable(tag, i));
        if (!m) continue;
        math::aabb<float> b = m->ComputeAbsoluteAccurateBoundingBox();
        if (!any) { box = b; any = true; }
        else { box.addInternalPoint(b.maxPoint); box.addInternalPoint(b.minPoint); }
    }
    if (!any) return;
    box.repair();
    math::rect<float> r(0, 0, 0, 0);
    GetProjectedAABB(*sm->GetCamera(), box, r);
    const math::vec2<unsigned int> &s = jet::video::Driver::GetCurrent()->GetCrtRenderTarget()->GetSize();
    if (r.lowerRightCorner.x > (float)s.x) r.lowerRightCorner.x = (float)s.x;
    if (r.lowerRightCorner.y > (float)s.y) r.lowerRightCorner.y = (float)s.y;
    if (r.upperLeftCorner.x < 0) r.upperLeftCorner.x = 0;
    if (r.upperLeftCorner.y < 0) r.upperLeftCorner.y = 0;
    if (r.lowerRightCorner.y < r.upperLeftCorner.y) r.upperLeftCorner.y = r.lowerRightCorner.y;
    if (r.lowerRightCorner.x < r.upperLeftCorner.x) r.upperLeftCorner.x = r.lowerRightCorner.x;
    ClearDepthBuffer(r);
}

void GameUtils::MenuRenderScene(jet::scene::SceneMgr *sm, unsigned tag, bool clearAllDepth, bool renderShadowMap) {
    if (!clearAllDepth) ClearDepthBuffer(sm, tag);
    else ClearDepthBuffer();
    if (!renderShadowMap) sm->ClearRenderables(GameLevel::GetShadowTagName());
    sm->Render();
}

void GetUniformsByName(jet::scene::Model *model, const jet::String &name, std::vector<ShaderUniformRef> &out) {
    if (!model) return;
    for (unsigned m = 0; m < model->GetMaterialCount(); m++) {
        jet::video::Material &mat = model->GetMutableMaterial(m);
        for (unsigned p = 0; p < mat.GetRenderPassCount(); p++) {
            int u = mat.FindUniformIdxByName(p, name);
            if (u >= 0) {
                ShaderUniformRef r = { &mat, p, u };
                out.push_back(r);
            }
        }
    }
}

jet::String GameUtils::IncreaseKey(const jet::String &key, int n) {
    std::string s(key.c_str());
    size_t cut = s.rfind('_');
    size_t start = cut == std::string::npos ? 0 : cut + 1;
    std::string tok = s.substr(start);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", atoi(tok.c_str()) + n);
    std::string num(buf);
    std::string out = s.substr(0, start);
    if (num.size() < tok.size()) out.append(tok.size() - num.size(), '0');
    return jet::String((out + num).c_str());
}

int GameUtils::ElementsCount(const jet::String &firstKey) {
    babel::StringMgr *mgr = babel::Babel::GetInstance()->GetStringMgr();
    int i = 0;
    while (mgr->Exists(IncreaseKey(firstKey, i))) i++;
    return i;
}

int GameUtils::GetLoadedLanguageAndroidIndex() {
    if (!babel::Babel::GetInstance()) return 0;
    switch (babel::Babel::GetInstance()->GetSelectedLanguage().value()) {
    case babel::Lang::FR: return 1;
    case babel::Lang::DE: return 2;
    case babel::Lang::IT: return 3;
    case babel::Lang::ES: return 4;
    case babel::Lang::JA: return 5;
    case babel::Lang::KO: return 6;
    case babel::Lang::ZH: return 7;
    case babel::Lang::PT:
    case babel::Lang::BR: return 8;
    case babel::Lang::RU: return 9;
    case babel::Lang::TR: return 10;
    default: return 0;
    }
}

// @0x3bd84c
std::vector<unsigned> GameUtils::IndexDistributedRandom(std::vector<float> &w, unsigned count, ControlledRandom *rnd) {
    std::vector<unsigned> out;
    const unsigned n = (unsigned)w.size();
    if (n < 2) {
        out.assign(count, 0u);
        return out;
    }
    float total = 0;
    for (float f : w) total += f;
    const float lo = total < 0 ? total : 0.0f, hi = total < 0 ? 0.0f : total;
    for (unsigned k = 0; k < count; k++) {
        float r = rnd ? rnd->Randf(0, total) : lo + (hi - lo) * (1.0f / 65536.0f) * (float)(jet::core::Rand() & 0xffff);
        unsigned pick = 0;
        for (unsigned i = 0; i < n; i++) {
            r -= w[i];
            if (!(r > 0)) {
                pick = i;
                break;
            }
        }
        for (unsigned j = 0; j < n; j++)
            if (j != pick) w[j] += (1.0f / (float)(n - 1)) * w[pick];
        w[pick] = 0;
        out.push_back(pick);
    }
    return out;
}

// @0x3bda10
std::vector<unsigned> GameUtils::IndexDistributedRandom(unsigned weightCount, unsigned count, ControlledRandom *rnd) {
    std::vector<float> w(weightCount, 100.0f / (float)weightCount);
    return IndexDistributedRandom(w, count, rnd);
}
