// source/game/entities/LensFlareDef.cpp
#include "LensFlareDef.h"
#include <sstream>
#include "param.h"
#include "sprites/sprite.h"
#include "sprites/spritemgr.h"
#include "video/texture.h"

jet::String LensFlareDef::k_tmplName("LensFlareDef"), LensFlareDef::k_LensFlareElement_tmplName("LensFlareElement");

// @0x5710b8: the elements of the "Elements" parameter: the sprite (loaded by SpriteMgr, its texture kept in memory at
// its best level), the frame its animation name holds, how it blends, "Distance" and "Scale"; then "Intensity"
void LensFlareDef::Init() {
    m_intensity = 0.5f;
    clara::DataEntity::Init();
    const clara::Param *elements = FindParamByName(jet::String("Elements"));
    for (unsigned i = 0; i < elements->GetComponentCount(); i++) {
        clara::DataEntity *e = elements->GetAsEntity(i);
        const clara::Param::SpriteValue &sv = e->FindParamByName(jet::String("Sprite"))->GetAsSprite(0);
        Sprite *sprite = SpriteMgr::GetInstance()->GetSprite(sv.file, false);
        if (!sprite) continue;
        if (boost::shared_ptr<jet::video::Texture> tex = sprite->GetTexture(0)) {
            tex->SetCanBeStreamed(false);
            tex->SetLod(0);
        }
        Element el;
        el.sprite = sprite;
        el.frame = 0;
        std::istringstream(std::string(sv.anim.c_str())) >> el.frame;
        const jet::String &blending = e->FindParamByName(jet::String("Blending"))->GetAsEnumName(0);
        if (blending.Equals("Additive")) el.blending = 1;
        else if (blending.Equals("Transparent")) el.blending = 2;
        else el.blending = blending.Equals("Multiply") ? 3 : 4;
        el.distance = 0;
        e->GetParam(jet::String("Distance"), el.distance, 0);
        el.scale = 0;
        e->GetParam(jet::String("Scale"), el.scale, 0);
        m_elements.push_back(el);
    }
    if (HasParam(jet::String("Intensity"))) GetParam(jet::String("Intensity"), m_intensity, 0);
}
