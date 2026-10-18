// source/game/interfaces/interfaceobjects/InterfaceFillRect.cpp
#include "InterfaceFillRect.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/utils/GameUtils.h"
#include "video/color.h"

const jet::String InterfaceFillRect::k_uiTmplName("FillRect");

// @0x44c43c
void InterfaceFillRect::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    unsigned a = (unsigned)l->GetParamValue(e, 11), r = (unsigned)l->GetParamValue(e, 12), g = (unsigned)l->GetParamValue(e, 13), b = (unsigned)l->GetParamValue(e, 14);
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)),
         math::vec2<float>((float)l->GetParamValue(e, 9), (float)l->GetParamValue(e, 10)),
         (b & 0xff) | (a << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8));
}

void InterfaceFillRect::Init(math::vec2<float> pos, math::vec2<float> size, unsigned argb) {
    SetPos(pos);
    SetSize(size);
    m_color = argb;
}

// @0x44c5a4
void InterfaceFillRect::Update(int dt) {
    if (!IsVisible()) return;
    IsActive();
    UpdateChildren(dt);
}

// @0x44c5ec: the rect at its combined position, cut by the clip
void InterfaceFillRect::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    math::vec2<float> pos = GetPos(), off = GetOffset(), size = GetSize();
    math::rect<float> r(pos.x + off.x, pos.y + off.y, pos.x + off.x + size.x, pos.y + off.y + size.y);
    if (HasClip()) r.clipAgainst(GetClip());
    jet::video::Color c(((m_color >> 16) & 0xff) / 255.0f, ((m_color >> 8) & 0xff) / 255.0f, (m_color & 0xff) / 255.0f, (m_color >> 24) / 255.0f);
    GameUtils::DrawRect(painter, r, c);
    RenderChildren(cam, painter);
}
