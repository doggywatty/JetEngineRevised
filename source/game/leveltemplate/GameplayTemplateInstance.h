// source/game/leveltemplate/GameplayTemplateInstance.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_GAMEPLAYTEMPLATEINSTANCE_H
#define SOURCE_GAME_LEVELTEMPLATE_GAMEPLAYTEMPLATEINSTANCE_H

#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/leveltemplate/LevelTemplateInstance.h"
#include "game/leveltemplate/SpeedDef.h"
#include "game/leveltemplate/gameplaytemplateinstancelistener.h"
#include "game/entities/PlatformTemplate.h"
#include "game/gameenums.h"
#include "paths/pathcommon.h"

class Object;
class PlatformInstance;
class CameraController;

// A level template of the run: the lanes along its ground platform, the platforms and the gameplay objects they make as the minion nears,
// its camera and speed and where it leads to
class GameplayTemplateInstance : public LevelTemplateInstance {  // sizeof 0x1E8
public:
    // one of the three lanes: the path along its dummies, the nodes where moving is forbidden or a bonus runs
    struct LaneData {
        bool valid, single;                         // 0x0, 0x1
        boost::shared_ptr<PathCommon> path;         // 0x4
        unsigned samplesPerSegment;                 // 0xC
        std::vector<bool> collisions, bonus;        // 0x10, 0x24
        std::vector<math::vec3<float> > positions;  // 0x38
        std::vector<math::quat<float> > rotations;  // 0x48
        LaneData() : valid(false), single(false), samplesPerSegment(0) {}
    };
    explicit GameplayTemplateInstance(LevelTemplateDef *def);
    virtual ~GameplayTemplateInstance();
    void RegisterListener(GameplayTemplateInstanceListener *l) { m_listeners.insert(l); }
    void UnregisterListener(GameplayTemplateInstanceListener *l) { m_listeners.erase(l); }
    void RenderLanes();
    void SetTemplateID(int id) { m_id = id; }
    int GetTemplateID() const { return m_id; }
    void SetCamera(GameEntity *c, int transitionMs) { m_camera = c; m_cameraTransitionTime = transitionMs; }
    GameEntity *GetCamera() const { return m_camera; }
    void SetSpeed(const SpeedDef *s);
    const SpeedDef *GetSpeed() const { return m_speed; }
    void SetLocation(const jet::String &l) { m_location = l; }
    const jet::String &GetLocation() const { return m_location; }
    CameraController *GetCameraController() const;
    // the template's camera becomes the level's, blended over ms (the template's own transition time when negative)
    void ApplyCameraValue(int ms);
    // @0x454e10: the speed the template asks for is the speed of the run (none: the speed of the speed manager)
    void ApplySpeedValue();
    // the shooting area (its class mask) and the location data of the template are the level's
    void ApplyGameplayValues();
    boost::shared_ptr<PathCommon> GetLanePath(LaneIndex l) const { return m_lanes[l].path; }
    bool IsSingleLane(LaneIndex l) const { return m_lanes[l].single; }
    bool IsValidLane(LaneIndex l) const { return m_lanes[l].valid; }
    unsigned GetNumSamplesPerSegment(LaneIndex l) const { return m_lanes[l].samplesPerSegment; }
    // whether moving from one lane to another is allowed at a node
    bool CanReachLane(LaneIndex from, LaneIndex to, int node) const;
    bool HasCollisionOnLane(LaneIndex l, unsigned node) const;
    void SetCollisionOnLane(LaneIndex l, unsigned node, bool b);
    bool IsBonusGameplayNode(LaneIndex l, unsigned node) const;
    void CacheLanesTransform(std::vector<math::vec3<float> > &pos, std::vector<math::quat<float> > &rot, std::vector<bool> &valid, unsigned segment, float t) const;
    bool GetClosestLanePosition(const math::vec3<float> &p, LaneIndex &lane, unsigned &node) const;
    bool GetClosestLanePosition(const math::vec3<float> &p, math::vec3<float> &pos, math::quat<float> &rot) const;
    bool IsBonusTemplate() const { return m_def->IsBonusTemplate(); }
    bool IsGameplayTemplate() const { return m_def->IsGameplayTemplate(); }
    bool IsPauseTemplate() const { return m_def->IsPauseTemplate(); }
    bool IsTransitionTemplate() const { return m_def->IsTransitionTemplate(); }
    float GetBeginMinionSledgeOffset() const;
    float GetEndMinionSledgeOffset() const;
    int GetSledgeType() const;
    virtual void Translate(const math::vec3<float> &d);
    Bifurcation GetBifurcation() const;
    bool HasSecretExit() const;
    Bifurcation GetSecretExitDirection() const;
    const std::vector<unsigned> &GetSecretExitLevelTemplates() const;
    virtual void GetEndAnchor(math::vec3<float> &pos, float &angle) const { GetBifurcation(); LevelTemplateInstance::GetEndAnchor(pos, angle); }
    // how far along the template a point is (the distance across its start, or by the closest lane node on a bifurcation)
    unsigned ComputeRelativeDistance(const math::vec3<float> &p) const;
    bool GetPredictedTransform(unsigned distance, math::vec3<float> &pos, math::quat<float> &rot) const;
    // places an entity of the deco generation group, giving where it is along the template
    void AddToEntityGenerationGroup(GameEntity::GenerationGroupEntity *e, unsigned index);
    void InitEntityGenerationGroup();
    void ClearEntityGenerationGroup();
    virtual void BuildAllowedDecoList(unsigned group) { LevelTemplateInstance::BuildAllowedDecoList(group ? group : m_entityGenerationGroupId); }
    void SendGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent&);
    virtual void SetEnabled(bool b);
    virtual bool IsEnabled() const { return m_isEnabled; }
    // creates the platforms' gameplay objects one by one as the minion nears
    void UpdateDelayedPlatformInstances();
    bool HasPendingPlatformInstances() const { return !m_platformInstances.empty(); }
    float GetStartDistance() const { return m_startDistance; }
    void SetStartDistance(float d) { m_startDistance = d; }
    unsigned GetNumTotalDummies() const { return m_numTotalDummies; }
    unsigned GetEntityGenerationGroupId() const { return m_entityGenerationGroupId; }
    void SetCreationNodeLabel(const jet::String &s) { m_creationNodeLabel = s; }
    const jet::String &GetCreationNodeLabel() const { return m_creationNodeLabel; }
    void SetCreationNodeId(const jet::String &s) { m_creationNodeId = s; }
    const jet::String &GetCreationNodeId() const { return m_creationNodeId; }
    void SetCreationNodeGraphId(int id) { m_creationNodeGraphId = id; }
    int GetCreationNodeGraphId() const { return m_creationNodeGraphId; }
    bool IsGeneratedBySpecialNode() const;
    Object *GetGroundObject() const { return m_groundObject; }
    static const jet::String k_emptyStr;
protected:
    virtual GameEntity *ProcessCreatedEntity(const GameEntity *tmpl, GameEntity *copy, GameplayTemplateInstance *previous);
private:
    static jet::String GetLaneDummyName(LaneIndex lane, int hundreds, int tens, int ones);
    void InitLanes(Object *obj, PlatformInstance *copy, PlatformInstance *groundDef, GameplayTemplateInstance *previous);
    void InitLaneForbiddenMovementData(const PlatformTemplate*);
    void InitBonusIntervalsData(const PlatformTemplate*);
    int m_id;                                    // 0x80
    float m_startDistance;                       // 0x84
    GameEntity *m_camera;                        // 0x88
    int m_cameraTransitionTime;                  // 0x8C
    SpeedDef *m_speed;                           // 0x90
    jet::String m_location;                      // 0x94
    LaneData m_lanes[3];                         // 0x98
    std::vector<LaneInterval*> m_laneForbiddenMovements;  // 0x1A0
    Object *m_groundObject;                      // 0x1AC
    std::set<GameplayTemplateInstanceListener*> m_listeners;  // 0x1B0
    std::vector<PlatformInstance*> m_platformInstances;  // 0x1C8
    unsigned m_numTotalDummies;                  // 0x1D4
    unsigned m_entityGenerationGroupId;          // 0x1D8
    jet::String m_creationNodeLabel;             // 0x1DC
    jet::String m_creationNodeId;                // 0x1E0
    int m_creationNodeGraphId;                   // 0x1E4
    static unsigned s_entityGenerationGroupSequentialId;
};

#endif
