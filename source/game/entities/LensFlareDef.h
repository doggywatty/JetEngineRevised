// source/game/entities/LensFlareDef.h
#ifndef SOURCE_GAME_ENTITIES_LENSFLAREDEF_H
#define SOURCE_GAME_ENTITIES_LENSFLAREDEF_H

#include <vector>
#include "entity.h"
#include "core/pstring.h"

class Sprite;

// What a lens flare is made of: the sprites along the line through the light and the middle of the screen, each with
// where it sits on it, how big it is and how it blends, and how bright they all are
class LensFlareDef : public clara::DataEntity {  // sizeof 0x30
public:
    struct Element {  // sizeof 0x14
        Sprite *sprite;  // 0x0
        unsigned frame;  // 0x4
        int blending;    // 0x8: the Sprite blending: 1 additive, 2 transparent, 3 multiply, 4 the others
        float distance;  // 0xC: along the line, 0 on the light and 1 on the middle of the screen
        float scale;     // 0x10
    };
    static jet::String k_tmplName, k_LensFlareElement_tmplName;
    explicit LensFlareDef(const clara::Template *t) : clara::DataEntity(t), m_intensity(0.5f) {}
    virtual void Init();
    const std::vector<Element> &GetElements() const { return m_elements; }
    float GetIntensity() const { return m_intensity; }
private:
    float m_intensity;                // 0x20
    std::vector<Element> m_elements;  // 0x24
};

#endif
