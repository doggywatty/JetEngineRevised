// source/game/entities/RenderPassDef.h
#ifndef SOURCE_GAME_ENTITIES_RENDERPASSDEF_H
#define SOURCE_GAME_ENTITIES_RENDERPASSDEF_H

#include <vector>
#include "GameEntity.h"

namespace jet::video { class Material; }

// One pass of a MaterialDef ("RenderPass" template, 0x90 bytes)
class RenderPassDef : public GameEntity {
public:
    explicit RenderPassDef(const clara::Template*);
    virtual ~RenderPassDef();
    void LoadMaterialValues(jet::video::Material&, unsigned) const;
    void LoadMaterialTextures(jet::video::Material&, std::vector<jet::String>&, unsigned) const;
    static jet::String k_tmplName;
};

#endif
