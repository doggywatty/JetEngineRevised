// source/game/interfaces/menus/box/IGTitleUIBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_IGTITLEUIBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_IGTITLEUIBOX_H

class IGTitleUIBox : public BasicPage {  // sizeof 0x218
public:
    IGTitleUIBox();
    virtual ~IGTitleUIBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void Show(const jet::String&);
protected:
    void SetState(safe_enum<IGTitleUIBox::EStateDef,enum IGTitleUIBox::EStateDef::type>);
    InterfaceText *m_titleIText;  // 0x1C4
    safe_enum<IGTitleUIBox::EStateDef,enum IGTitleUIBox::EStateDef::type> m_state;  // 0x1C8
    static const int k_introDelayerTime;
    static math::vec2<float> k_introOffset;
    jet::core::Delayer<math::vec2<float>,jet::core::SmoothStep> m_introDelayer;  // 0x1CC
    static const int k_outroDelayerTime;
    static math::vec2<float> k_outroOffset;
    jet::core::Delayer<math::vec2<float>,jet::core::SmoothStep> m_outroDelayer;  // 0x1F0
    static const int k_showTimer;
    int m_showTimer;  // 0x214
};

#endif
