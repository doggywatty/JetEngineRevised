// source/game/interfaces/menus/box/TauntItemBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_TAUNTITEMBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_TAUNTITEMBOX_H

class TauntItemBox : public BasicPage {  // sizeof 0x1D4
public:
    TauntItemBox(TauntDef*);
    virtual ~TauntItemBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetState(safe_enum<TauntItemBox::EStateDef,enum TauntItemBox::EStateDef::type>);
    void SetEquipped(bool);
    const jet::String &GetTauntClaraName();
protected:
    safe_enum<TauntItemBox::EStateDef,enum TauntItemBox::EStateDef::type> m_state;  // 0x1C4
    TauntDef *m_tauntDef;  // 0x1C8
    InterfaceButton *m_buyButton;  // 0x1CC
    InterfaceButton *m_useButton;  // 0x1D0
};

#endif
