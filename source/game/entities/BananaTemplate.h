// source/game/entities/BananaTemplate.h
#ifndef SOURCE_GAME_ENTITIES_BANANATEMPLATE_H
#define SOURCE_GAME_ENTITIES_BANANATEMPLATE_H

#include "Object.h"
#include "Deco3d.h"

// A banana (the data's "BananaTemplate", type 0x15; 0x118 bytes in the .so): an object the minion collects when it comes within the
// collection distance. It only updates while the minion is near (30 m), shows another deco while the banana multiplier is on, and goes
// through the states 0 alive, 1 collected, 2 gone. The magnet (PowerUpMgr) takes the bananas within its distance and pulls them to the minion.
// The sounds, the bananas counted (Statistics::AddBananas) and what the minion is told when it collects one in a rocket (Minion::OnBananaCollect,
// the rocket is not ported) join with the sound manager and the rocket.
class BananaTemplate : public Object {
public:
    struct BananaState { enum { Alive = 0, Collected = 1, Gone = 2 }; };
    explicit BananaTemplate(const clara::Template*);
    virtual ~BananaTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void OnLinkedHidden() { m_state = BananaState::Collected; }
    virtual void OnMinionRevive();
    // @0x5a05f4: a banana of the pool is made ready again (shown, in its first state)
    void PoolInstance_Prepare(bool pooled);
    void Die();
    void SetMagnetized(bool);
    void Push(const math::vec2<float>&) {}
    int GetState() const { return m_state; }
    bool IsMagnetized() const { return m_isMagnetized; }
    static jet::String k_tmplName;
    static const jet::String k_comboDeco, k_collectionSound, k_collectionComboSound, k_collectionDistance, k_magnetDistance;
private:
    bool IsBananaMultiplierActive() const;
    // @0x5a0740 (the magnet's pull is part of it: the .so has no symbol of its own for it)
    void UpdateBananaState();
    void CheckBananaDecoChange();
    int m_state;                          // 0xC8
    float m_magnetizedTime;               // 0xCC
    bool m_isMagnetized;                  // 0xD0
    bool m_comboState;                    // 0xD1: shows the combo deco
    Deco3d::TModelData m_origData;        // 0xD4
    Deco3d::TModelData m_comboData;       // 0xEC
    float m_collectionDistanceSq;         // 0x104
    float m_magnetDistanceSq;             // 0x108
    jet::String m_collectionComboSoundLabel;  // 0x10C
    jet::String m_collectionSoundLabel;   // 0x110
    bool m_pooled;                        // 0x114
};

#endif
