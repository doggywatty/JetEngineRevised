// source/game/interfaces/interfaceobjects/Interface3DCostumePos.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DCOSTUMEPOS_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACE3DCOSTUMEPOS_H

#include "InterfaceObject.h"

class AuroraLevelData;
class MinionCostume;

// A minion costume standing where the element is on screen ("3DCostumePos", 0x1C0 bytes in the .so): a copy of a costume
// entity (the equipped one, or the library costume named by the element) in the state of the animixer name (Idle), drawn
// in the menu scene a little turned to the side, with the effects its states start (EffectMgr, ModelPool; not their sounds).
class Interface3DCostumePos : public InterfaceObject {
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    Interface3DCostumePos();
    Interface3DCostumePos(AuroraLevelData*, int);
    virtual ~Interface3DCostumePos();
    void Init(math::vec2<float> pos, float scale, const jet::String &animixerState, const jet::String &costumeLibName, bool renderShadowDeco);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void UnloadClaraData() {}
    virtual void Reset() {}
    virtual void OnMenuQuit();
    virtual void SetScale(const math::vec2<float> &s) { SetScaleDirect(s); }
    virtual void SetScale(float s) { SetScaleDirect(math::vec2<float>(s, s)); }
    void SetAnimixerState(const jet::String&);
    void EnableShadowDeco(bool e) { m_renderShadowDeco = e; }
    MinionCostume *GetMinionCostume() const { return m_minionCostume; }
    static const jet::String k_uiTmplName, k_Minion_Costume_State_Name_Idle;
    static const math::mat3<float> &CostumeRotation();
private:
    void _CheckMinionReload();
    void _ClearMinionData();
    jet::String m_animixerStateName;     // 0x1A4
    jet::String m_minionCostumeLibName;  // 0x1A8: empty for the equipped costume
    MinionCostume *m_minionCostume;      // 0x1AC
    bool m_renderShadowDeco;             // 0x1B0
    math::vec3<float> m_shadowDecoScale; // 0x1B4
};

#endif
