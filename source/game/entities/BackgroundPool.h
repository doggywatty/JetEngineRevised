// source/game/entities/BackgroundPool.h
#ifndef SOURCE_GAME_ENTITIES_BACKGROUNDPOOL_H
#define SOURCE_GAME_ENTITIES_BACKGROUNDPOOL_H

#include <vector>
#include "entity.h"
#include "clara.h"

// The backgrounds usable in an environment
class BackgroundPool : public clara::DataEntity {  // sizeof 0x38
public:
    static const jet::String k_tmplName, k_environmentTypeParam, k_backgroundDataPoolParam;
    explicit BackgroundPool(const clara::Template *t) : clara::DataEntity(t), m_environmentType(0) {}
    virtual void PostInit();
    int GetEnvironmentType() const { return m_environmentType; }
    const std::vector<clara::Path> &GetBackgroundDataPaths() const { return m_backgrounds; }
private:
    int m_environmentType;                 // 0x20
    std::vector<clara::Path> m_backgrounds;  // 0x24
};

#endif
