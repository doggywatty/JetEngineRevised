// source/game/interfaces/interfaceobjects/InterfaceObject.cpp
#include "InterfaceObject.h"
#include <cstdlib>
#include <cstring>
#include <typeinfo>
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/entities/LevelDef.h"
#include "game/bodypartdetection/BodyPartOwner.h"
#include "scene/node.h"
#include "scene/scenemgr.h"
#include "game/gamemanagers/PainterMgr.h"
#include "sprites/fontmgr.h"
#include "sprites/sprite.h"
#include "stream/istream.h"
#include "stream/streammgr.h"
#include "system.h"
#include "video/display.h"
#include "video/driver.h"
#include <cmath>
#include <memory>

bool InterfaceObject::s_debugEnabled = false;
float InterfaceObject::s_3DAspectRatioScale = 1.0f;
float InterfaceObject::s_autoScrollText_animScale = 0.0f;
std::vector<int> InterfaceObject::s_palettesColor;
std::vector<int> InterfaceObject::s_palettesOutlineColor;
std::vector<Interface3DLayer> InterfaceObject::s_customI3DLayer;
std::vector<Interface3DLayer> InterfaceObject::s_renderI3DLayerStack;

// @0x44fb20
InterfaceObject::InterfaceObject()
    : m_visible(true), m_active(true), m_parent(0), m_layerIdx(-1), m_clip(0, 0, 0, 0), m_rotation(0),
      m_oldText2DWorldCoords(false), m_guiLevel(0), m_guiId(-1), m_guiParentId(-1), m_pos(0, 0), m_offset(0, 0), m_size(0, 0),
      m_spriteArtOffset(0, 0), m_scale(1, 1), m_UIScale(1, 1) {
    if (s_customI3DLayer.empty()) s_customI3DLayer.resize(5);
}

// @0x44fc94: leaves its parent and releases its children
InterfaceObject::~InterfaceObject() {
    if (m_parent) {
        m_parent->RemoveChild(this);
        m_parent = 0;
    }
    while (!m_childObjects.empty()) m_childObjects.front()->SetParent(0);
}

// @0x44fd74: element id and parent from the Aurora layer
void InterfaceObject::Init() {
    if (!m_guiLevel) return;
    m_guiParentId = m_guiLevel->GetElementParentId(m_guiId);
    m_layerIdx = m_guiLevel->GetParamValue(m_guiId, 1);
}

void InterfaceObject::Update(int) {}
void InterfaceObject::UnloadClaraData() {}
void InterfaceObject::Reset() {}
void InterfaceObject::OnMenuQuit() {}
void InterfaceObject::UpdateChildOffsets() {}
void InterfaceObject::_Debug_Render(jet::video::Painter&) {}
unsigned InterfaceObject::_Debug_RenderColor() { return 0xffff00ff; }

// @0x44fdac
void InterfaceObject::Render(const jet::scene::Camera&, jet::video::Painter &p) {
    if (!IsVisible()) return;
    RenderDebug(p);
}

// @0x45012c
void InterfaceObject::RenderDebug(jet::video::Painter &p) {
    if (s_debugEnabled) _Debug_Render(p);
}

// @0x44fde4
void InterfaceObject::UpdateChildren(int dt) {
    for (size_t i = 0; i != m_childObjects.size(); i++) {
        InterfaceObject *c = m_childObjects[i];
        if (c->IsVisible() && c->IsActive()) c->Update(dt);
    }
}

// @0x44fe68
void InterfaceObject::RenderChildren(const jet::scene::Camera &cam, jet::video::Painter &p) {
    for (size_t i = 0; i != m_childObjects.size(); i++) {
        InterfaceObject *c = m_childObjects[i];
        if (const char *sk = getenv("JET_SKIPTYPE")) if (strstr(typeid(*c).name(), sk)) continue;
        if (c->IsVisible()) c->Render(cam, p);
    }
}

void InterfaceObject::SetVisible(bool v) { m_visible = v; }
void InterfaceObject::SetActive(bool a) { m_active = a; }
void InterfaceObject::SetRotation(float r) { m_rotation = r; }
bool InterfaceObject::IsVisible() { return (!m_parent || m_parent->IsVisible()) && m_visible; }
bool InterfaceObject::IsActive() { return (!m_parent || m_parent->IsActive()) && m_active; }

void InterfaceObject::SetUIScale(float s) { m_UIScale = math::vec2<float>(s, s); }
void InterfaceObject::SetScale(float s) { m_scale = math::vec2<float>(s, s); }
void InterfaceObject::SetScale(const math::vec2<float> &s) { m_scale = s; }
const math::vec2<float> &InterfaceObject::GetScale() { return m_scale; }

// @0x3cf1fc
math::vec2<float> InterfaceObject::GetCombinedScale() {
    math::vec2<float> s = GetUIScale();
    return math::vec2<float>(s.x * m_scale.x, s.y * m_scale.y);
}

// @0x450220
math::vec2<float> InterfaceObject::GetUIScale() {
    if (!m_parent) return m_UIScale;
    math::vec2<float> s = m_parent->GetUIScale();
    return math::vec2<float>(s.x * m_UIScale.x, s.y * m_UIScale.y);
}

void InterfaceObject::SetPos(math::vec2<float> p) { m_pos = p; }
void InterfaceObject::SetOffset(const math::vec2<float> &o) { m_offset = o; }
void InterfaceObject::SetSize(const math::vec2<float> &s) { m_size = s; }
void InterfaceObject::SetSpriteArtOffset(const math::vec2<float> &o) { m_spriteArtOffset = o; }

math::vec2<float> InterfaceObject::GetPos() {
    math::vec2<float> s = GetUIScale();
    return math::vec2<float>(s.x * m_pos.x, s.y * m_pos.y);
}

math::vec2<float> InterfaceObject::GetUnscaledPos() { return m_pos; }

// @0x3cf2bc
math::vec2<float> InterfaceObject::GetCombinedPos() {
    math::vec2<float> p = GetPos(), o = GetOffset(), a = GetSpriteArtOffset();
    return math::vec2<float>(p.x + o.x + a.x, p.y + o.y + a.y);
}

// @0x4501b0
math::vec2<float> InterfaceObject::GetOffset() {
    if (!m_parent) return m_offset;
    math::vec2<float> o = m_parent->GetOffset();
    return math::vec2<float>(o.x + m_offset.x, o.y + m_offset.y);
}

const math::vec2<float> &InterfaceObject::GetOffsetNoParent() { return m_offset; }
math::vec2<float> InterfaceObject::GetOffsetForClip() { return GetOffset(); }

math::vec2<float> InterfaceObject::GetSize() {
    math::vec2<float> s = GetUIScale();
    return math::vec2<float>(s.x * m_size.x, s.y * m_size.y);
}

const math::vec2<float> &InterfaceObject::GetUnscaledSize() { return m_size; }

// @0x3cf3c0: scaled by the combined (not the UI) scale
math::vec2<float> InterfaceObject::GetSpriteArtOffset() {
    math::vec2<float> s = GetCombinedScale();
    return math::vec2<float>(s.x * m_spriteArtOffset.x, s.y * m_spriteArtOffset.y);
}

// @0x4515c4: inside the display (edges included)
bool InterfaceObject::IsScreenPosVisible(math::vec2<float> p) {
    math::vec2<unsigned int> size = jet::video::Driver::GetCurrent()->GetDisplay()->GetSize();
    if (p.x < 0 || p.y < 0) return false;
    return (unsigned)p.x <= size.x && (unsigned)p.y <= size.y;
}

// @0x450288
void InterfaceObject::SetParent(InterfaceObject *p) {
    if (m_parent) m_parent->RemoveChild(this);
    m_parent = p;
    if (p) p->AddChild(this);
}

// @0x4402d4
void InterfaceObject::AddChild(InterfaceObject *c) {
    m_childObjects.push_back(c);
    UpdateChildOffsets();
}

// @0x450330
void InterfaceObject::RemoveChild(InterfaceObject *c) {
    for (size_t i = 0; i != m_childObjects.size(); i++)
        if (m_childObjects[i] == c) {
            m_childObjects.erase(m_childObjects.begin() + i);
            UpdateChildOffsets();
            return;
        }
}

// @0x45053c
int InterfaceObject::GetChildIdx(InterfaceObject *c) {
    for (size_t i = 0; i != m_childObjects.size(); i++)
        if (m_childObjects[i] == c) return (int)i;
    return -1;
}

// @0x4404e4: a clip of its own or of an ancestor
bool InterfaceObject::HasClip() {
    if (m_parent && m_parent->HasClip()) return true;
    return !(m_clip.lowerRightCorner.x == m_clip.upperLeftCorner.x && m_clip.lowerRightCorner.y == m_clip.upperLeftCorner.y);
}

// @0x4503a8: the own clip (moved by its offset) inside the parent's
math::rect<float> InterfaceObject::GetClip() {
    if (!m_parent || !m_parent->HasClip()) {
        math::vec2<float> o = GetOffsetForClip();
        math::rect<float> r = m_clip;
        r.upperLeftCorner += o;
        r.lowerRightCorner += o;
        return r;
    }
    if (m_clip.lowerRightCorner.x == m_clip.upperLeftCorner.x && m_clip.lowerRightCorner.y == m_clip.upperLeftCorner.y) return m_parent->GetClip();
    math::rect<float> parent = m_parent->GetClip();
    math::vec2<float> o = GetOffsetForClip();
    math::rect<float> r = m_clip;
    r.upperLeftCorner += o;
    r.lowerRightCorner += o;
    parent.clipAgainst(r);
    return parent;
}

// @0x4411a8
void InterfaceObject::SetScaleForChildren(const math::vec2<float> &s) {
    for (size_t i = 0; i != m_childObjects.size(); i++) m_childObjects[i]->SetScale(s);
}

// @0x44fed8: the layer goes on the stack, the first one also swaps the sprite material for the unlit 3D one
void InterfaceObject::PushI3DTransform(const Interface3DLayer &layer) {
    s_renderI3DLayerStack.push_back(layer);
    _Apply3dTransform(layer.pos(), layer.edgeA(), layer.edgeB(), layer.size());
    if (s_renderI3DLayerStack.size() == 1) {
        m_oldMaterial = Sprite::GetMaterial();
        Sprite::SetOverridenRenderTechnique(jet::String("deco3d"));
        Sprite::GetMaterial().GetRenderState(0).SetDepthTest(false);
        Sprite::GetMaterial().GetRenderState(0).SetDepthWrite(false);
        m_oldText2DWorldCoords = FontMgr::Is2DRenderingWorldCoords();
        FontMgr::Set2DRenderingWorldCoords(true);
    }
}

// @0x450078
void InterfaceObject::PopI3DTransform() {
    if (s_renderI3DLayerStack.empty()) return;
    s_renderI3DLayerStack.pop_back();
    if (s_renderI3DLayerStack.empty()) {
        PainterMgr::GetInstance()->Get2DPainter().SetPostClipTransform(m_oldPainterTransform);
        Sprite::GetMaterial() = m_oldMaterial;
        FontMgr::Set2DRenderingWorldCoords(m_oldText2DWorldCoords);
    } else {
        const Interface3DLayer &l = s_renderI3DLayerStack.back();
        _Apply3dTransform(l.pos(), l.edgeA(), l.edgeB(), l.size());
    }
}

// @0x450574: the quad's matrix: translation * rotation * scale (edge lengths over the authored size)
void InterfaceObject::_Apply3dTransform(const math::vec3<float> &center, const math::vec3<float> &edgeA, const math::vec3<float> &edgeB, const math::vec2<float> &size) {
    jet::video::Painter &painter = PainterMgr::GetInstance()->Get2DPainter();
    m_oldPainterTransform = painter.GetPostClipTransform();
    math::vec3<float> a = edgeA, b = edgeB;
    float lenB = b.getLength(), lenA = a.getLength();
    b.normalize();
    a.normalize();
    math::vec3<float> n = math::cross(b, a);
    n.normalize();
    math::quat<float> rot;
    rot.setFromMat3(math::lookAt(a, n));
    math::mat4<float> t;
    t.setTranslation(center);
    math::mat4<float> r;
    r.setRotationPart(rot.getAsMat3());
    math::mat4<float> s;
    float sx = lenB / size.x, sy = lenA / size.y;
    s.m[0] *= sx; s.m[1] *= sx; s.m[2] *= sx;
    s.m[4] *= sy; s.m[5] *= sy; s.m[6] *= sy;
    painter.SetPostClipTransform(t * r * s);
}

// @0x450784: the rect as projected through the transform when 3D layers are active
math::rect<float> InterfaceObject::GetTransformedRect(const jet::scene::Camera &camera, const math::mat4<float> &m, const math::rect<float> &r) {
    if (s_renderI3DLayerStack.empty()) return r;
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(camera);
    math::vec3<float> a = math::transform(m, math::vec3<float>(r.upperLeftCorner.x, r.upperLeftCorner.y, 0));
    math::vec3<float> b = math::transform(m, math::vec3<float>(r.lowerRightCorner.x, r.lowerRightCorner.y, 0));
    cam.Project(a, a);
    cam.Project(b, b);
    return math::rect<float>(a.x, a.y, b.x, b.y);
}

// @0x440880
bool InterfaceObject::Update3DLayerTransform(Interface3DLayer &l, const math::vec3<float> &center, const math::vec3<float> &edgeA, const math::vec3<float> &edgeB, const math::vec2<float> &size) {
    l.pos() = center;
    l.edgeA() = edgeA;
    l.edgeB() = edgeB;
    l.size() = size;
    return true;
}

// @0x4408e8: a quad of the given scale (x width, z height) rotated by rot, placed by its center
bool InterfaceObject::Update3DLayerTransform(Interface3DLayer &l, const math::vec3<float> &pos, const math::quat<float> &rot, const math::vec3<float> &scale, const math::vec2<float> &size) {
    const math::vec3<float> axisA(0, 0, 1), axisB(-1, 0, 0);
    math::vec3<float> a = rot * math::vec3<float>(scale.x * axisA.x, scale.y * axisA.y, scale.z * axisA.z);
    math::vec3<float> b = rot * math::vec3<float>(scale.x * axisB.x, scale.y * axisB.y, scale.z * axisB.z);
    math::vec3<float> sum = b + a;
    return Update3DLayerTransform(l, pos + math::vec3<float>(sum.x * 0.5f, sum.y * 0.5f, sum.z * 0.5f), a, b, size);
}

// @0x440a20: the layer follows a body part of the owner: the part's cylinder (or sphere) in world
// space, scaled by how far the node's scale has moved from the model's base scale
bool InterfaceObject::Update3DLayerTransform(Interface3DLayer &l, BodyPartOwner *bpo, unsigned idx, const math::vec2<float> &size) {
    if (!bpo || idx >= bpo->BPO_GetBodyPartCount()) return false;
    BodyPartSection *s = bpo->BPO_GetBodyPart(idx);
    const BodyPartDefinition *d = s->def;
    const jet::scene::Node *n = s->node;
    const math::quat<float> &nq = n->GetAbsoluteRotation();
    math::vec3<float> np = n->GetAbsolutePosition();
    math::vec3<float> pos = np + nq * d->position;
    if (d->height != 0.0f) {
        math::vec3<float> b = pos + (nq * d->rotation) * math::vec3<float>(0, 0, d->height);
        pos = (pos + b) * 0.5f;
    }
    math::quat<float> rot = nq * d->rotation;
    float dia = d->radius * 2.0f;
    math::vec3<float> sc(dia, dia, d->height);
    math::vec3<float> right = rot * jet::scene::SceneMgr::GetRightVector();
    math::vec3<float> front = rot * jet::scene::SceneMgr::GetFrontVector();
    rot = math::quat<float>(math::lookAt(right, math::vec3<float>(-front.x, -front.y, -front.z)));
    const math::vec3<float> &bs = n->GetData()->scale, &cs = n->GetScale();
    math::vec3<float> scale((cs.x / bs.x) * sc.z, (cs.z / bs.z) * sc.x, (cs.y / bs.y) * sc.y);
    return Update3DLayerTransform(l, pos, rot, scale, size);
}

// @0x440ed4: the inverse of the above
void InterfaceObject::Get3DLayerTransfrom(const Interface3DLayer &l, math::vec3<float> &pos, math::quat<float> &rot, math::vec3<float> &scale, math::vec2<float> &size) {
    math::vec3<float> a = l.edgeA(), b = l.edgeB();
    float lenB = b.getLength(), lenA = a.getLength();
    scale = math::vec3<float>(lenB, 1.0f, lenA);
    pos = l.pos();
    a.normalize();
    b.normalize();
    math::vec3<float> n = math::cross(a, b);
    n.normalize();
    rot.setFromMat3(math::lookAt(n, a));
    size = l.size();
}

// @0x440ffc: "<count>, count x {color, outline color}"
void InterfaceObject::LoadPalettesValues(const jet::String &name) {
    s_palettesColor.clear();
    s_palettesOutlineColor.clear();
    std::unique_ptr<jet::stream::IStream> s(jet::stream::StreamMgr::GetInstance().CreateStream(name));
    if (!s) return;
    s->Start();
    if (s->IsStarted()) {
        int count = 0;
        *s >> count;
        for (int i = 0; i < count; i++) {
            int v = 0;
            *s >> v;
            s_palettesColor.push_back(v);
            *s >> v;
            s_palettesOutlineColor.push_back(v);
        }
    }
    s->Finish();
}

int InterfaceObject::GetPaletteOutlineColor(int i) { return (unsigned)i < s_palettesOutlineColor.size() ? s_palettesOutlineColor[i] : 0; }
int InterfaceObject::GetPaletteColor(int i) { return (unsigned)i < s_palettesColor.size() ? s_palettesColor[i] : 0; }

// @0x4411f4: where the ray through the screen point meets the plane facing the camera through planePoint
math::vec3<float> InterfaceObject::MenuModel_Get3DPos(const jet::scene::Camera &camera, const math::vec2<float> &screen, const math::vec3<float> &planePoint) {
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(camera);
    math::vec3<float> front = cam.GetFrontVector();
    float d = -math::dot(planePoint, front);
    math::line3<float> ray = cam.GetRayFromScreenCoordinates(screen);
    math::vec3<float> dir = ray.end - ray.start;
    float denom = math::dot(front, dir);
    if (denom == 0.0f) return planePoint;
    float t = -(math::dot(front, ray.start) + d) / denom;
    return ray.start + dir * t;
}

// @0x441384: at the minion costume distance in front of the camera
math::vec3<float> InterfaceObject::MenuModel_Get3DPos(const jet::scene::Camera &camera, const math::vec2<float> &screen) {
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(camera);
    float dist = LevelDef::GetInstance()->GetI3DMinionCostumeCamDistance();
    math::vec3<float> front = cam.GetFrontVector();
    math::vec3<float> p = cam.GetPosition() + math::vec3<float>(front.x * dist, front.y * dist, front.z * dist);
    math::vec3<float> projected(0, 0, 0);
    cam.Project(projected, p);
    math::vec3<float> out(0, 0, 0);
    cam.Unproject(out, math::vec3<float>(screen.x, screen.y, projected.z));
    return out;
}

// @0x441464
math::mat3<float> InterfaceObject::MenuModel_FaceCameraPlane(const jet::scene::Camera &camera, const math::vec3<float> &target) {
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(camera);
    math::vec3<float> right = cam.GetRightVector();
    math::vec3<float> toCam = cam.GetPosition() - target;
    toCam.normalize();
    math::vec3<float> back(-toCam.x, -toCam.y, -toCam.z);
    math::vec3<float> up = math::cross(right, back);
    math::mat3<float> m;
    m.m[0] = right.x; m.m[1] = right.y; m.m[2] = right.z;
    m.m[3] = back.x; m.m[4] = back.y; m.m[5] = back.z;
    m.m[6] = up.x; m.m[7] = up.y; m.m[8] = up.z;
    return m;
}

// @0x451564: text longer than its box waits 3 s, then scrolls for 3 s
void InterfaceObject::UpdateAutoScrollText() {
    s64 t = jet::System::GetTime() % k_textAutoScroll_totalTime;
    s_autoScrollText_animScale = t < k_textAutoScroll_pauseTime + 1 ? 0.0f : (float)(t - k_textAutoScroll_pauseTime) / (float)k_textAutoScroll_scrollTime;
}
