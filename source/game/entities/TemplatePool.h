// source/game/entities/TemplatePool.h
#ifndef SOURCE_GAME_ENTITIES_TEMPLATEPOOL_H
#define SOURCE_GAME_ENTITIES_TEMPLATEPOOL_H

#include <vector>
#include "entity.h"

// The level templates usable in an environment
class TemplatePool : public clara::DataEntity {  // sizeof 0x34
public:
    static const jet::String k_tmplName, k_environmentTypeParam, k_templateIdPoolParam;
    explicit TemplatePool(const clara::Template *t) : clara::DataEntity(t), m_environmentType(0) {}
    virtual void PostInit();
    int GetEnvironmentType() const { return m_environmentType; }
    const std::vector<int> &GetTemplateIds() const { return m_templateIds; }
private:
    int m_environmentType;         // 0x20
    std::vector<int> m_templateIds;  // 0x24
};

#endif
