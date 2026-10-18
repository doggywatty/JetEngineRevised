// source/game/entities/MaterialDef.cpp
#include "MaterialDef.h"
#include "RenderPassDef.h"
#include <cstring>

jet::String MaterialDef::k_tmplName("Material");
bool MaterialDef::s_postFxEnabled = true;
std::vector<jet::String> MaterialDef::s_noTextureNames;

MaterialDef::MaterialDef(const clara::Template *t) : GameEntity(t), m_isMaterialLoaded(false) {}
MaterialDef::~MaterialDef() {}

void MaterialDef::Init() {
    GameEntity::Init();
    LoadMaterial();
}

void MaterialDef::LoadMaterial() {
    if (m_isMaterialLoaded) return;
    LoadMaterial(m_material, m_textureNames);
    m_isMaterialLoaded = true;
}

// LoadMaterial @0x573618: technique from "Shader", then each "Passes" entity's values and
// textures, then the draw layer. Without post effects a postfx_ technique becomes basic.xml.
void MaterialDef::LoadMaterial(jet::video::Material &m, std::vector<std::vector<jet::String> > &names) const {
    jet::String shader;
    GetParam(jet::String("Shader"), shader);
    bool skipPasses = false;
    if (!s_postFxEnabled && strstr(shader.c_str(), "postfx_")) {
        shader = jet::String("basic.xml");
        skipPasses = true;
    }
    m.SetRenderTechnique(shader);
    names.clear();
    if (!m.GetRenderTechnique()) return;
    names.resize(m.GetRenderPassCount());
    if (!skipPasses)
        if (const clara::Param *p = FindParamByName(jet::String("Passes")))
            for (unsigned i = 0; i < p->GetComponentCount() && i < m.GetRenderPassCount(); i++) {
                const RenderPassDef *pass = dynamic_cast<const RenderPassDef*>(p->GetAsEntity(i));
                if (!pass) continue;
                pass->LoadMaterialValues(m, i);
                pass->LoadMaterialTextures(m, names[i], i);
            }
    int layer = 0;
    if (GetParam(jet::String("DrawLayer"), layer)) m.SetDrawLayer(layer);
}

const jet::video::Material &MaterialDef::GetMaterial() const { return m_material; }
jet::video::Material &MaterialDef::GetMaterial() { return m_material; }
const std::vector<jet::String> &MaterialDef::GetAllTextureNames(unsigned pass) const {
    return pass < m_textureNames.size() ? m_textureNames[pass] : s_noTextureNames;
}
