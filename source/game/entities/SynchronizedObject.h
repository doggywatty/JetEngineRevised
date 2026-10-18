// source/game/entities/SynchronizedObject.h
#ifndef SOURCE_GAME_ENTITIES_SYNCHRONIZEDOBJECT_H
#define SOURCE_GAME_ENTITIES_SYNCHRONIZEDOBJECT_H

#include <vector>
#include "pacesetterfollower.h"
#include "Pacesetter.h"
#include "core/pstring.h"
#include "smath.h"

class Object;

// What an object does in time with the run (0x88 bytes in the .so, the object holding it passes itself to Init): either it plays its
// "PreAnchorPositionAnimation" at the time the minion needs to reach it (so a door opens as the minion comes), or it moves along a
// pacesetter of its own for "syncTime" ms to meet the minion at its place ("useSyncMovement"), and it can tell the minion's engine
// event "preContactEngineEvent" a few ms before the contact.
class SynchronizedObject : public PacesetterFollower {
public:
    // what Init found in the object's data (m_animType, +0x20)
    struct SynchronizationAnimType { enum { None = 0, PrePosition = 1, SyncMovement = 2 }; };
    SynchronizedObject();
    virtual ~SynchronizedObject();
    void Init(Object*);
    void Update();
    virtual void OnSyncFinished() {}                      // @0x518a78
    void StopPacesetter(bool relink);
    virtual void GoTo(const math::vec3<float>&, const math::quat<float>&) {}  // @0x522dfc
    virtual void OnGameplayTemplateInstanceDestroyed();
    LaneIndex GetSpawnLane() const { return m_spawnLane; }
    void SetSpawnLane(LaneIndex lane) { m_spawnLane = lane; }  // the inline of InteractiveObjectTemplate::SetSpawnLane @0x563d04
    bool OnMinionRevive();
    // the pacesetter of a sync movement (null when the object only plays its pre position animation)
    Pacesetter *GetOwnPacesetter() const { return m_pacesetterInstance; }
protected:
    void ResetPacestterCourse();
    void AddSyncMovementDistance(float d);
    float ComputeTimeToGetReached();
private:
    void InitPacesetter();
    bool InitPacesetterCourse();
    bool StartPacestterCourse(float timeToReach, float distance);
    void _Update();
    void _UpdatePreContact();
    std::vector<Pacesetter::PacestterCacheData> m_cache0;  // 0x08: where the minion's lane was last looked at for ComputeTimeToGetReached
    std::vector<Pacesetter::PacestterCacheData> m_cache1;  // 0x14: and for the distance of the own pacesetter
    int m_animType;                                        // 0x20
    jet::String m_prePositionAnimation;                    // 0x24
    jet::String m_postPositionAnimation;                   // 0x28
    int m_timeOffset;                                      // 0x2C
    int m_prePositionAnimationTime;                        // 0x30: the duration of the pre position animation
    int m_lastTime;                                        // 0x34: the time to reach it found last (ms)
    int m_skipCount;                                       // 0x38: frames to let go before the next _Update
    Pacesetter *m_pacesetterInstance;                      // 0x3C: the own pacesetter of a sync movement
    int m_syncTime;                                        // 0x40
    float m_syncDistance;                                  // 0x44
    bool m_pacesetterCourseInit;                           // 0x48
    bool m_pacesetterCourseFinished;                       // 0x49
    bool m_pacesetterCourseLocked;                         // 0x4A
    float m_timeToGetReached;                              // 0x4C
    float m_distanceToGetReached;                          // 0x50
    int m_lastTimeComputeFrame;                            // 0x54
    LaneIndex m_spawnLane;                                 // 0x58
    Object *m_owner;                                       // 0x5C
    math::vec3<float> m_localPosition;                     // 0x60: where the owner's deco was before the pacesetter took it
    math::quat<float> m_localRotation;                     // 0x6C
    int m_preContactTimeMax;                               // 0x7C
    int m_preContactTimeMin;                               // 0x80
    int m_preContactEngineEvent;                           // 0x84
    static const jet::String k_prePositionAnimation, k_postPositionAnimation, k_timeOffset, k_useSyncMovement, k_syncDistance, k_syncTime,
        k_preContactTimeMaxParam, k_preContactTimeMinParam, k_preContactEngineEventParam;
};

#endif
