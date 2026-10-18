// source/game/interfaces/interfaceobjects/InterfaceArea.cpp
#include "InterfaceArea.h"
#include "game/interfaces/aurora/AuroraLevelData.h"

const jet::String InterfaceArea::k_uiTmplName("Area");

// @0x44a334: x y from parameters 2 3, size from 9 10
void InterfaceArea::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)),
         math::vec2<float>((float)l->GetParamValue(e, 9), (float)l->GetParamValue(e, 10)));
}

void InterfaceArea::Init(math::vec2<float> pos, math::vec2<float> size) {
    SetPos(pos);
    SetSize(size);
}

// @0x44a428
void InterfaceArea::Update(int dt) {
    if (!IsVisible()) return;
    IsActive();
    UpdateChildren(dt);
}

// @0x44a470
void InterfaceArea::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    InterfaceObject::Render(cam, p);
    if (!IsVisible()) return;
    RenderChildren(cam, p);
}
