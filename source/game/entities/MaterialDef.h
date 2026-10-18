// source/game/entities/MaterialDef.h
#ifndef SOURCE_GAME_ENTITIES_MATERIALDEF_H
#define SOURCE_GAME_ENTITIES_MATERIALDEF_H

#include <vector>
#include "GameEntity.h"
#include "video/material.h"

// "Material" template (0x198 bytes): a technique plus RenderPass entities with values/textures
class MaterialDef : public GameEntity {
public:
    explicit MaterialDef(const clara::Template*);
    virtual ~MaterialDef();
    virtual void Init();
    const jet::video::Material &GetMaterial() const;
    jet::video::Material &GetMaterial();
    const std::vector<jet::String> &GetAllTextureNames(unsigned pass) const;
    // Builds the material into m (textures per pass into names); used for models' own materials
    void LoadMaterial(jet::video::Material &m, std::vector<std::vector<jet::String> > &names) const;

    static jet::String k_tmplName;
    // Game flag (+0x3ac of the game object): when off, "postfx_" techniques become basic.xml
    static bool s_postFxEnabled;
private:
    void LoadMaterial();
    jet::video::Material m_material;                      // 0x90
    std::vector<std::vector<jet::String> > m_textureNames;  // 0x180
    bool m_isMaterialLoaded;                              // 0x194
    static std::vector<jet::String> s_noTextureNames;
};

#endif
