// source/game/gamemanagers/FlareMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_FLAREMGR_H
#define SOURCE_GAME_GAMEMANAGERS_FLAREMGR_H

#include <vector>
#include "jsingleton.h"
#include "smath.h"

class LensFlareEntity;

// The lens flares of the background in view: each fades in once the sun is seen from the camera
class FlareMgr : public Singleton<FlareMgr> {  // sizeof 0x28
public:
    struct Flare {  // sizeof 0x3C
        LensFlareEntity *entity = 0;      // 0x0
        float from = 0, current = 0, to = 0;  // 0x4 0x8 0xC
        int fadeTime = 5000;              // 0x10
        int elapsed = 0;                  // 0x14
        bool fading = false;              // 0x18
        bool occluded = true;             // 0x1C: something is in the way of the light (the last ray cast hit)
        math::vec3<float> position;       // 0x20
        math::quat<float> rotation;       // 0x2C
    };
    FlareMgr();
    virtual ~FlareMgr() { CleanFlares(); }
    void Enable(bool b) { m_enabled = b; }
    void AddFlare(LensFlareEntity *e, const math::vec3<float> &pos, const math::quat<float> &rot);
    void CleanFlares();
    // every period frames: which flares have the light in sight of the camera
    void Update(unsigned ms);
    void Render();
private:
    void SortFlaresByDistance();
    std::vector<Flare*> m_flares;  // 0x4
    bool m_enabled;                // 0x14
    int m_frame, m_period;         // 0x18 0x1C
    unsigned m_ms;                 // 0x20
    int m_visible;                 // 0x24: how many are in sight
};

#endif
