// source/game/entities/BonusForCostumes.h
#ifndef SOURCE_GAME_ENTITIES_BONUSFORCOSTUMES_H
#define SOURCE_GAME_ENTITIES_BONUSFORCOSTUMES_H

#include <vector>
#include "entity.h"

// A costume's bonus ("BonusForCostumes" data entity, 0x40 bytes in the .so): a skill (1 free revives, 3 longer power-up
// of an item type, 4 bananas factor, 5 bananas on friend challenges, 8 easier friend challenges, ...) and its amount;
// a location dependent bonus only counts in its locations.
class BonusForCostumes : public clara::DataEntity {
public:
    explicit BonusForCostumes(const clara::Template*);
    virtual ~BonusForCostumes();
    virtual void Init() {}  // @0x5a3db0
    virtual void PostInit();
    int GetSkillType() const;
    float GetAmountAsFloat() const { return m_amount; }
    int GetAmountAsInt() const { return (int)m_amount; }
    int GetGameItemType() const { return m_gameItemType; }
    static jet::String k_tmplName;
private:
    float m_amount;                 // 0x20
    int m_skillType;                // 0x24
    int m_gameItemType;             // 0x28
    bool m_isLocationDependent;     // 0x2C
    std::vector<int> m_locations;   // 0x30
};

#endif
