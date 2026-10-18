// source/game/interfaces/menus/Menu_SendChallengeTaunt.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_SENDCHALLENGETAUNT_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_SENDCHALLENGETAUNT_H

// See Menu_Shop.h for why these additions were needed.
#include "Menu_Base.h"
#include "game/online/tracking/tracking.h"
class TauntItemBox;

class Menu_SendChallengeTaunt : public Menu_Base {  // sizeof 0x160
public:
    // Not in the PDB header, and not recovered from disassembly (not critical path -- this is an
    // internal-only element type for m_tauntElements, never touched by this session's minimal,
    // render-only .cpp). An empty placeholder shape is enough to make std::vector<STauntElement>
    // compile; do NOT treat this as a verified layout if a later session starts actually using it.
    struct STauntElement {};
    virtual const jet::String &GetMenuName() const;
    Menu_SendChallengeTaunt();
    virtual ~Menu_SendChallengeTaunt();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
    void SetEquipped(TauntItemBox*);
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
private:
    void LoadCostumes();
    void FreeCostumes();
    void UpdateCostumes(int);
    void RenderCostumes();
    bool SendChallenge();
public:
    static const jet::String k_menuName;
private:
    InterfaceButton *m_sendChallengeButton;  // 0x124
    InterfaceButton *m_backButton;  // 0x128
    std::vector<Menu_SendChallengeTaunt::STauntElement,std::allocator<Menu_SendChallengeTaunt::STauntElement> > m_tauntElements;  // 0x12C
    math::vec3<float> m_selectedCostumePos;  // 0x140
    math::quat<float> m_selectedCostumeRot;  // 0x14C
    unsigned m_selectedTauntId;  // 0x15C
};

#endif
