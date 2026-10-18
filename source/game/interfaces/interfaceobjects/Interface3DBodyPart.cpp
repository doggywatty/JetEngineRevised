// source/game/interfaces/interfaceobjects/Interface3DBodyPart.cpp
#include "Interface3DBodyPart.h"
#include "Interface3DTouchInteract.h"
#include "game/bodypartdetection/BodyPartOwner.h"
#include "game/entities/TouchInteract3d.h"
#include "game/interfaces/aurora/AuroraLevelData.h"

const jet::String Interface3DBodyPart::k_uiTmplName("3DBodyPart");

// @0x4486e4: position from parameters 2 3, size from 9 10, 3D layer from 0xc, body part from 0xe
void Interface3DBodyPart::Init() {
    InterfaceObject::Init();
    AuroraLevelData *l = m_guiLevel;
    int e = m_guiId;
    Init(math::vec2<float>((float)l->GetParamValue(e, 2), (float)l->GetParamValue(e, 3)), l->GetParamValue(e, 9), l->GetParamValue(e, 10), l->GetParamValue(e, 0xc), l->GetParamValue(e, 0xe));
}

void Interface3DBodyPart::Init(math::vec2<float> pos, int w, int h, int layer3DId, int bodyPart) {
    SetPos(pos);
    SetSize(math::vec2<float>((float)w, (float)h));
    Layer3DId_Set(layer3DId);
    m_modelBodyPart = bodyPart;
    m_initialLayer3DId = m_layer3DId;
}

// the body part owner of the parent element's entity
BodyPartOwner *Interface3DBodyPart::GetParentBodyPartOwner() const {
    if (!m_parent || !m_parent->GetUITemplateName().Equals(Interface3DTouchInteract::k_uiTmplName)) return 0;
    return static_cast<Interface3DTouchInteract*>(m_parent)->GetTouchInteractEntity();
}

void Interface3DBodyPart::Update(int dt) {
    if (!IsVisible()) return;
    IsActive();
    UpdateChildren(dt);
}

// @0x4488e0
void Interface3DBodyPart::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    if (!m_useCustom3DLayerData) {
        if (m_layer3DId == 0) {
            if (BodyPartOwner *bpo = GetParentBodyPartOwner()) Update3DLayerTransform(m_i3DLayerData, bpo, (unsigned)m_modelBodyPart, GetSize());
        } else {
            m_i3DLayerData = GetCustomI3DLayer(m_layer3DId);
        }
    }
    if (getenv("JET_DBGBP")) fprintf(stderr, "bp vis %d layer %d custom %d idx %d owner %p children %u pos %.2f %.2f %.2f eA %.2f %.2f %.2f eB %.2f %.2f %.2f\n", IsVisible(), m_layer3DId, m_useCustom3DLayerData, m_modelBodyPart, (void*)GetParentBodyPartOwner(), GetChildCount(), m_i3DLayerData.pos().x, m_i3DLayerData.pos().y, m_i3DLayerData.pos().z, m_i3DLayerData.edgeA().x, m_i3DLayerData.edgeA().y, m_i3DLayerData.edgeA().z, m_i3DLayerData.edgeB().x, m_i3DLayerData.edgeB().y, m_i3DLayerData.edgeB().z);
    if (getenv("JET_DBGBP")) for (unsigned i = 0; i < GetChildCount(); i++) { InterfaceObject *ch = GetChild((int)i); fprintf(stderr, "  child %u vis %d act %d size %.0fx%.0f cpos %.0f,%.0f nch %u\n", i, ch->IsVisible(), ch->IsActive(), ch->GetSize().x, ch->GetSize().y, ch->GetCombinedPos().x, ch->GetCombinedPos().y, ch->GetChildCount()); for (unsigned j = 0; j < ch->GetChildCount(); j++) { InterfaceObject *g = ch->GetChild((int)j); fprintf(stderr, "    gchild %u vis %d size %.0fx%.0f cpos %.0f,%.0f\n", j, g->IsVisible(), g->GetSize().x, g->GetSize().y, g->GetCombinedPos().x, g->GetCombinedPos().y); } }
    PushI3DTransform(m_i3DLayerData);
    InterfaceObject::Render(cam, painter);
    RenderChildren(cam, painter);
    PopI3DTransform();
}

// @0x4489f0
void Interface3DBodyPart::UpdateVisibilityFromBPO() {
    if (m_useCustom3DLayerData || m_layer3DId != 0) return;
    BodyPartOwner *bpo = GetParentBodyPartOwner();
    if (!bpo || (unsigned)m_modelBodyPart >= bpo->BPO_GetBodyPartCount()) return;
    SetVisible((bpo->BPO_GetBodyPart((unsigned)m_modelBodyPart)->flags & BodyPartSection::Visible) != 0);
}
