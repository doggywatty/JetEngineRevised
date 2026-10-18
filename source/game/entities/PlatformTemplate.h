// source/game/entities/PlatformTemplate.h
#ifndef SOURCE_GAME_ENTITIES_PLATFORMTEMPLATE_H
#define SOURCE_GAME_ENTITIES_PLATFORMTEMPLATE_H

#include <vector>
#include "GameEntity.h"
#include "ObjectHolder.h"
#include "../gameenums.h"

class Object;

// The nodes of a lane between which a move to another lane is not allowed (or a bonus runs), in lane indices
struct LaneInterval {
    int originLane, destinationLane;
    math::vec2<float> nodeInterval;
};

// What a platform of a level template is made of: the object whose model holds the lanes' dummies, how the run bifurcates there, its
// secret exit and the lane movements it forbids
class PlatformTemplate : public GameEntity, public ObjectHolder {  // sizeof 0xE0
public:
    static jet::String k_tmplName, k_LaneInterval_tmplName;
    explicit PlatformTemplate(const clara::Template *t);
    virtual ~PlatformTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void SetPersistenceType(int t);
    virtual void SetKeepCreatedEntity(bool b) { m_keepCreatedEntity = b; }
    virtual Object *GetCreatedEntity() const { return m_createdEntity; }
    GameEntity *GetDecoReference() const;
    GameEntity *GetCollisionReference() const;
    const std::vector<LaneInterval*> &GetForbiddenMovements() const { return m_laneForbiddenMovements; }
    const std::vector<LaneInterval*> &GetBonusIntervals() const { return m_bonusIntervals; }
    Bifurcation GetBifurcation() const { return m_bifurcation; }
    void GetTeeLimits(unsigned &left, unsigned &right) const { left = m_teeLeft; right = m_teeRight; }
    bool HasSecretExit() const { return m_hasSecretExit; }
    Bifurcation GetSecretExitDirection() const { return m_secretExitDirection; }
    const std::vector<unsigned> &GetSecretExitLevelTemplates() const { return m_levelTemplateList; }
    float GetBeginMinionSledgeOffset() const { return m_beginMinionSledgeOffset; }
    float GetEndMinionSledgeOffset() const { return m_endMinionSledgeOffset; }
    int GetSledgeType() const { return m_sledgeType; }
private:
    void LoadIntervals(const clara::Param*, std::vector<LaneInterval*>&);
    static Bifurcation ParseBifurcation(const jet::String&, Bifurcation current, bool allowTee);
    Object *m_createdEntity;                      // 0x8C
    bool m_keepCreatedEntity;                     // 0x90
    Bifurcation m_bifurcation;                    // 0x94
    bool m_hasSecretExit;                         // 0x98
    Bifurcation m_secretExitDirection;            // 0x9C
    std::vector<unsigned> m_levelTemplateList;    // 0xA0: the templates a secret exit leads to
    bool m_hasForbiddenMovements;                 // 0xAC
    std::vector<LaneInterval*> m_laneForbiddenMovements;  // 0xB0
    bool m_hasBonusIntervals;                     // 0xBC
    std::vector<LaneInterval*> m_bonusIntervals;  // 0xC0
    float m_beginMinionSledgeOffset;              // 0xCC
    float m_endMinionSledgeOffset;                // 0xD0
    int m_sledgeType;                             // 0xD4
    unsigned m_teeLeft, m_teeRight;               // 0xD8, 0xDC
};

#endif
