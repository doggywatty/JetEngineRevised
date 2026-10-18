// source/game/entities/TouchInteract3d.h
#ifndef SOURCE_GAME_ENTITIES_TOUCHINTERACT3D_H
#define SOURCE_GAME_ENTITIES_TOUCHINTERACT3D_H

#include <vector>
#include "GameEntity.h"
#include "game/GameLevel.h"
#include "game/bodypartdetection/BodyPartOwner.h"

class Deco3d;

// What a touch on a TouchInteract3d asks of the main menu (TouchInteractEvents)
struct TouchInteractEvents { enum type { None = 0, Event1 = 1, Event2 = 2, Event3 = 3, Event4 = 4, Hide = 5 }; };

// "touchInteract3d" template (TouchInteract3d, 0x148): a touchable 3d menu element shown by a copy of its
// objectDeco, with body parts that react to touches by playing the deco's pressed / touched animations and
// asking the main menu for an event (Init @0x4b95f0, InitDeco3d @0x4b986c, Update @0x4b9bb0).
// Not ported yet: the touch sound, effect and despicable points (SoundMgr, EffectMgr, Minion).
class TouchInteract3d : public GameEntity, public BodyPartOwner {
public:
    explicit TouchInteract3d(const clara::Template*);
    TouchInteract3d(TouchInteract3d*, unsigned ctorFlags);
    virtual ~TouchInteract3d();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void SetVisible(bool);
    virtual void SetEnabled(bool);
    virtual void NotifyTransformChanged(bool);
    virtual void BPO_OnBodyPartTouched(BodyPartSection*);
    virtual void BPO_OnBodyPartPressed(BodyPartSection*);
    virtual void BPO_OnBodyPartReleased(BodyPartSection*);
    virtual void BPO_OnBodyPartDragged(BodyPartSection*) {}

    Deco3d *GetDeco() const { return m_deco; }
    void Reset(bool fromMenuQuit);
    void AutoHide();
    void SetSceneMgr(GameLevel::SceneMgrId);
    void SetModelNodesVisible(const jet::String &prefix, bool);
    void SetBPSVisibleIfItHas(int event, bool);
    void ScaleUIDecoAndBP(float);
    bool IsDecoAnimationFinished(const jet::String &anim);
    void SetDecoAnimation(const jet::String &anim, bool loop, bool force);
    void ExecuteTouchEvent(int event, bool outro);
    unsigned GetEffectLayerId() const { return m_effectLayerId; }

    static jet::String k_tmplName;
    static unsigned s_nextEffectLayer;
private:
    void InitDeco3d();
    void RandomizeAnims();
    void LoadEnumStringArray(const jet::String &param, std::vector<jet::String> &out) const;

    Deco3d *m_deco;                             // 0xd4
    std::vector<jet::String> m_idleAnims;       // 0xd8
    std::vector<jet::String> m_introAnims;      // 0xe4
    std::vector<jet::String> m_autoHideAnims;   // 0xf0
    jet::String m_idleAnim;                     // 0xfc  the chosen ones
    jet::String m_introAnim;                    // 0x100
    jet::String m_introSound;                   // 0x104
    jet::String m_autoHideAnim;                 // 0x108
    clara::DataEntity *m_onTouchEffect;         // 0x10c
    float m_despicablePoints;                   // 0x110
    math::vec3<float> m_decoScale;              // 0x114
    jet::String m_touchedAnim;                  // 0x120 the animation a touch started, until it ends
    int m_touchEvent;                           // 0x124 the event it asks for
    unsigned m_effectLayerId;                   // 0x12c
    unsigned m_ctorFlags;                       // 0x128
};

#endif
