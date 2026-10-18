// source/game/entities/BananaTemplatePath.h
#ifndef SOURCE_GAME_ENTITIES_BANANATEMPLATEPATH_H
#define SOURCE_GAME_ENTITIES_BANANATEMPLATEPATH_H

#include <memory>
#include <vector>
#include "GameEntity.h"
#include "paths/pathcommon.h"

class BananaTemplate;

// A line of bananas along a path (the data's "BananaTemplatePath", 0xAC bytes in the .so): the path runs through the points the entity holds
// and the bananas, as many as it asks for, are spread along it, linked to the entity so they follow it. It can project them on the ground.
class BananaTemplatePath : public GameEntity {
public:
    explicit BananaTemplatePath(const clara::Template*);
    virtual ~BananaTemplatePath();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    unsigned GetNumObjectsToCreate();
    virtual void OnLinkedHidden();
    virtual void NotifyTransformChanged(bool);
    static jet::String k_tmplName;
    static const jet::String k_points, k_pathWidth, k_interpType, k_numSamples, k_tension, k_bias, k_numObjectsInPath, k_projectOnGround;
private:
    void BuildPath();
    void CreateBananas();
    bool RefreshBananaTransform();
    bool ProjectOnGround(math::vec3<float>&);
    bool m_updateTransform;                       // 0x8C
    bool m_projectOnGround;                       // 0x8D
    std::unique_ptr<PathCommon> m_path;           // 0x90
    std::vector<BananaTemplate*> m_bananas;       // 0x94
};

#endif
