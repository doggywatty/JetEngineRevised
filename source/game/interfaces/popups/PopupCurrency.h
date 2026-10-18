// source/game/interfaces/popups/PopupCurrency.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPCURRENCY_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPCURRENCY_H

class PopupCurrency : public BasePopup {  // sizeof 0x234
public:
    static void ShowPopup(safe_enum<EPriceTypeDef,enum EPriceTypeDef::type>, int, TRACKING_SCREEN);
    PopupCurrency(safe_enum<EPriceTypeDef,enum EPriceTypeDef::type>, int, TRACKING_SCREEN);
    virtual ~PopupCurrency();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
private:
    safe_enum<EPriceTypeDef,enum EPriceTypeDef::type> m_priceType;  // 0x1C8
    int m_amountRequired;  // 0x1CC
    TRACKING_SCREEN m_redirection;  // 0x1D0
    InterfaceButton *m_closeButton;  // 0x1D4
    InterfaceButton *m_shopButton;  // 0x1D8
    InterfaceButton *m_buyBananasButton;  // 0x1DC
    int m_bananasPriceInTokens;  // 0x1E0
    std::vector<InterfaceButton *,std::allocator<InterfaceButton *> > m_buyTokensButtons;  // 0x1E4
    std::vector<math::rect<float>,std::allocator<math::rect<float> > > m_buyTokensIconAreasRects;  // 0x1F8
    std::vector<Buyable *,std::allocator<Buyable *> > m_iapBuyables;  // 0x20C
    std::vector<InterfaceGraph *,std::allocator<InterfaceGraph *> > m_waitIcons;  // 0x220
    static const int k_numTokensPacksShown;
};

#endif
