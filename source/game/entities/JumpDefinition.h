// source/game/entities/JumpDefinition.h
#ifndef SOURCE_GAME_ENTITIES_JUMPDEFINITION_H
#define SOURCE_GAME_ENTITIES_JUMPDEFINITION_H

#include <memory>
#include <vector>
#include "GameEntity.h"
#include "paths/pathcommon.h"
#include "core/interputils.h"

// Where a jump is along its path: the time in it, its total, the average front speed, and the place (segment and fraction, the one before too)
struct JumpDefinitionControl {
    float time = 0, total = 0, averageFrontSpeed = 0;  // 0x0, 0x4, 0x8
    float pgt = 0;                                      // 0xC: how far along the path in 0..1
    unsigned segment = 0;                               // 0x10
    float t = 0;                                        // 0x14
    float prevPgt = 0;                                  // 0x18
    unsigned prevSegment = 0;                           // 0x1C
    float prevT = 0;                                    // 0x20
};

// A jump the minion makes (the "JumpDefinition" template): a path of points, and how fast it is gone along in time
class JumpDefinition : public GameEntity {  // sizeof 0xD0
public:
    explicit JumpDefinition(const clara::Template*);
    virtual ~JumpDefinition();
    virtual void Init();
    virtual void PostInit();
    // moves the control on by dt; true when the end (or the start going back) is reached
    bool UpateControl(JumpDefinitionControl&, float dt);
    // how far the last update moved along the path
    math::vec3<float> GetMovementDelta(JumpDefinitionControl&);
    void InitCtrl(JumpDefinitionControl&);
    // the speed in front is cut or stretched toward the given one, inside the multiples of the definition
    void InitCtrl(JumpDefinitionControl&, float frontSpeed);
    float GetAverageFrontSpeed(JumpDefinitionControl&);
    bool GetCanFallOut() const { return m_canFallOut; }
    static jet::String k_tmplName;
    static jet::String k_JumpPathPoint_tmplName;
private:
    struct DistancePoint { float timePgt, distancePgt; };
    float TimeToPgt(float time, float total);
    void BuildPath();
    int m_interpolationType;                 // 0x8C
    float m_totalTime;                       // 0x90
    std::unique_ptr<PathCommon> m_path;      // 0x94
    float m_timeMultMin, m_timeMultMax;      // 0x98, 0x9C
    bool m_canFallOut;                       // 0xA0
    std::vector<DistancePoint> m_distanceDistribution;  // 0xA4
};

#endif
