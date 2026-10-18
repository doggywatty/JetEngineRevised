// source/game/bodypartdetection/BodyPartOwner.h
#ifndef SOURCE_GAME_BODYPARTDETECTION_BODYPARTOWNER_H
#define SOURCE_GAME_BODYPARTDETECTION_BODYPARTOWNER_H

#include <vector>
#include "smath.h"
#include "core/pstring.h"

namespace jet::scene { class Model; class Node; class Camera; }
namespace clara { class Entity; class Param; }
class GameEntity;

// A hit region on a model node, loaded from a clara body part entity (BodyPartDefinition, 0x54 bytes)
struct BodyPartDefinition {
    BodyPartDefinition();
    jet::String name;                // 0x00 entity name
    jet::String nodeRef;             // 0x04 NodeRef
    float radius;                    // 0x08
    float height;                    // 0x0C
    float angle;                     // 0x10 half angle in radians (Angle * pi/360)
    bool cylinderCover;              // 0x14
    math::vec3<float> position;      // 0x18 entity position
    math::quat<float> rotation;      // 0x24 euler(0, -pi/2, 0) * entity rotation
    int type;                        // 0x34
    int touchEvent;                  // 0x38
    jet::String touchedSound;        // 0x3C
    jet::String touchedAnimation;    // 0x40
    jet::String pressedAnimation;    // 0x44
    int linked3dLayer;               // 0x48 (linked3dLayer - 1, -1 when absent)
    float baseRadius;                // 0x4C copy of radius
    float baseHeight;                // 0x50 copy of height
};

// A definition bound to its model node (BodyPartSection, 0x18 bytes). Flags: 1 enabled, 2 visible,
// 4 and 8 touch state.
struct BodyPartSection {
    enum { Enabled = 1, Visible = 2, Check4 = 4, Check8 = 8 };
    BodyPartSection() : node(0), flags(0), def(0) { hit[0] = hit[1] = hit[2] = 0; }
    bool IsReadyToCheck() const;
    void GetStartAndEnd(math::vec3<float> &start, math::vec3<float> &end) const;
    void GetStart(math::vec3<float> &start) const;
    jet::scene::Node *node;          // 0x00
    unsigned char flags;             // 0x04
    BodyPartDefinition *def;         // 0x08
    float hit[3];                    // 0x0C
};

class BodyPartOwner {
public:
    enum BodyPartOwnerClassType { ClassType_Default = 1 };

    BodyPartOwner();
    virtual ~BodyPartOwner();
    void BPO_Init(jet::scene::Model*);
    void BPO_Enable(bool);
    void BPO_Release();
    void BPO_SetModel(jet::scene::Model*);
    void BPO_Update(int);
    void BPO_Render();
    virtual void BPO_OnBodyPartPressed(BodyPartSection*) = 0;
    virtual void BPO_OnBodyPartTouched(BodyPartSection*) = 0;
    virtual void BPO_OnBodyPartDragged(BodyPartSection*) = 0;
    virtual void BPO_OnBodyPartReleased(BodyPartSection*) = 0;
    void BPO_LoadBodyPartsArray(std::vector<BodyPartDefinition>&, clara::Param*);
    void BPO_AddBodyPartsArray(std::vector<BodyPartDefinition>&);
    void BPO_LoadBodyPart(BodyPartDefinition*, clara::Entity*);
    void BPO_AddBodyPart(BodyPartDefinition*);
    void BPO_RemoveBodyPart(BodyPartDefinition*);
    void BPO_SetBodyPartEnabled(BodyPartDefinition*, bool);
    void BPO_SetBodyPartVisible(BodyPartDefinition*, bool visible, bool setCheck, bool check);
    void BPO_AddToLastHit(BodyPartSection*);
    bool BPO_IsLastHit(BodyPartSection *s) const;
    unsigned BPO_GetLastHitCount() const { return (unsigned)m_lastHit.size(); }
    BodyPartSection *BPO_GetLastHit(unsigned i) { return m_lastHit[i]; }
    void BPO_ClearBodyPartHits() { m_lastHit.clear(); }
    unsigned BPO_GetBodyPartCount() const { return (unsigned)m_bodyParts.size(); }
    BodyPartSection *BPO_GetBodyPart(unsigned i) { return &m_bodyParts[i]; }
    std::vector<BodyPartDefinition> &BPO_GetDefinitions() { return m_bodyPartDefinitions; }
    void BPO_SetSceneMgr(int id) { m_sceneMgrId = id; }
    void BPO_Scale(float);
    virtual GameEntity *BPO_GetOwnerGameEntity() { return 0; }
    void SetClassType(BodyPartOwnerClassType t) { m_classType = t; }
    BodyPartOwnerClassType GetClassType() const { return m_classType; }
    math::vec3<float> BPO_GetHitSpherePos() const { return m_hitSpherePos; }
    int BPO_GetBodyPartHit(const jet::scene::Camera&, math::line3<float>, math::vec3<float> &outHitPos, math::quat<float> &outHitRot, bool addToLastHit);
    static BodyPartOwner *BPO_GetProjectileHit(const jet::scene::Camera&, const math::vec3<float> &rayStart, const math::vec3<float> &rayEnd, math::vec3<float> &outHitPos, math::quat<float> &outHitRot, BodyPartOwner *skipOwner, bool clearHits, bool addToLastHit, unsigned char ownerClassMask);
    static const std::vector<BodyPartOwner*> &BPO_GetDamageables() { return s_damageables; }
protected:
    virtual void BPO_RenderBodyPart(BodyPartSection*) {}
public:
    static jet::String k_SimpleBodyPartDefinition_tmplName;
    static jet::String k_TouchInteractBodyPartDefinition_tmplName;
protected:
    BodyPartOwnerClassType m_classType;   // 0x04
    float m_hitSphereRadius;              // 0x08
    math::vec3<float> m_hitSpherePos;     // 0x0C
private:
    jet::scene::Model *m_pModel;          // 0x1C
    std::vector<BodyPartSection> m_bodyParts;  // 0x20
    std::vector<BodyPartDefinition> m_bodyPartDefinitions;  // 0x2C
    std::vector<BodyPartSection*> m_lastHit;  // 0x38
    BodyPartOwner *m_pOwner;              // 0x44
    int m_sceneMgrId;
    static std::vector<BodyPartOwner*> s_damageables;
};

#endif
