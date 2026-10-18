// source/game/interfaces/menus/Menu_BaseCostumes.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_BASECOSTUMES_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_BASECOSTUMES_H

#include <vector>
#include "Menu_Base.h"

class MinionCostume;
class CostumeShopItem;
class BodyPartOwner;
class InterfaceScrollBar;
class InterfaceGraph;
class SpritePlayer;
struct Touch;

// What the main menu and the shop have in common: the wheel of costumes. Every costume is shown twice, as a big copy
// (the one in the middle, in the 3D scene) and as a small one in the strip under it; touching the strip or the middle
// scrolls them, the costume the wheel stops at is the current one and shows its panel (CostumeShopItem).
// All the positions are in the 3D scene: the costumes stand on the y axis, which is the screen's horizontal one.
// Not ported: the sound of the scroll (no SoundMgr here).
class Menu_BaseCostumes : public Menu_Base {
public:
    explicit Menu_BaseCostumes(const jet::String &guiLevelName);
    virtual ~Menu_BaseCostumes();
    using Menu_Base::Init;
    // the grid of the big costumes and the one of the index (the small ones), the panel of the selected costume and the
    // scroll bar: elements of the page
    virtual void Init(const jet::String &grid, const jet::String &indexGrid, const jet::String &selected, const jet::String &scrollBar);
    virtual void UpdateCostumes(int dt, bool, bool);
    virtual void RenderCostumes(const jet::scene::Camera&, jet::video::Painter&, bool smallOnly);
    virtual void ClearCostumes();
    virtual void RefreshCostumes();
    // where the costumes stand: the feet of the big one and of the next one, the same for the small ones
    void InitCostumes(const jet::String &bigFeet, const jet::String &bigNextFeet, const jet::String &smallFeet, const jet::String &smallNextFeet);
    void ReloadCostumes();
    MinionCostume *GetCurrentCostume() const;
    void SetItemsButtonsActive(bool);
    void SetCurrentCostume(int);
    void SetCostumeGUIVisible(bool v) { m_costumeGUIVisible = v; }
    bool IsScrollingCostumes() const;
    void Set3DTouchesDisableTimer(int ms) { m_3dTouchesDisableTimer = ms; }
    bool Are3DTouchesEnabled() const { return m_3dTouchesDisableTimer <= 0; }
    void CheckForNewCostumes() { m_reloadPending = true; }
    bool CanScrollCostumes(int);
    void ScrollCostumes(int);
    bool CurrentCostume_WasBought() const;
    void OnNextEnterSelectEquippedCostume() { m_currentPos = 0; }

    // the order of the shop: the awarded ones, then the bought ones first, the cheaper ones first
    struct CostumesComparison {
        bool operator()(MinionCostume*, MinionCostume*) const;
        bool m_equippedFirst;
    };
    static const int k_minTouchMoveToSwipe, k_maxTouchTimeToSwipe;
    static const jet::String k_sfx_costumes_scroll;

protected:
    void UpdateTouchesForCostumes(int dt, bool);
    bool CheckCostumesScrollBoundaries(float&);
    void ApplyOffset(bool touching, bool);
    void Update3DTouches(const Touch*);
    BodyPartOwner *GetTouchedBPOFor(const math::vec2<float>&);
    void CurrentCostume_CheckPlaySpecificAnim();

    std::vector<MinionCostume*> m_costumes;       // 0x124: the big ones
    std::vector<MinionCostume*> m_smallCostumes;  // 0x130: the ones of the strip
    int m_currentCostume;                         // 0x13C: index, -1 before the first load
    bool m_draggingGrid, m_draggingIndex;         // 0x140, 0x141: which of the two the finger holds
    bool m_currentWasBought;                      // 0x142: the bought animation was played for the current one
    math::vec3<float> m_bigPos;                   // 0x144: where the big costume of index 0 stands (y is the first one's)
    float m_bigSpacing;                           // 0x150: the distance in y between two big ones
    float m_currentPos;                           // 0x154: the strip's y of the current costume (0 asks for the equipped one)
    CostumeShopItem *m_item;                      // 0x158: the panel of the current costume
    InterfaceScrollBar *m_scrollBar;              // 0x15C
    InterfaceGraph *m_selectedGraph;              // 0x160
    math::rect<float> m_gridRect, m_indexRect;    // 0x164, 0x174
    float m_bigWorldPerPixel;                     // 0x184: the world y of a screen pixel at the big costumes
    bool m_locked;                                // 0x188
    int m_effectLayerId;                          // 0x18C
    math::vec3<float> m_smallPos;                 // 0x190: the same for the small ones
    SpritePlayer *m_badgeNew, *m_badgeTime, *m_badgePromo, *m_badgeBought, *m_badgePuzzle, *m_badgeAward, *m_badgeAwardSale;  // 0x19C..0x1B4
    math::vec3<float> m_badgeOffset;              // 0x1B8: from a small costume to where its badge is drawn
    float m_smallScreenX;                         // 0x1C4: the screen x of the small ones' feet
    float m_scrollOffset;                         // 0x1C8: the wheel's position, 0 at the first costume, in strip distances
    float m_smallSpacing;                         // 0x1CC: the distance in y between two small ones
    float m_halfSmallSpacing;                     // 0x1D0
    float m_smallWorldPerPixel;                   // 0x1D4
    int m_touchTime;                              // 0x1D8: ms the finger is down
    int m_transitionTimer;                        // 0x1DC: ms since the wheel stopped, to show the panel
    bool m_costumeGUIVisible;                     // 0x1E0
    BodyPartOwner *m_touchedBPO;                  // 0x1E4
    int m_3dTouchesDisableTimer;                  // 0x1E8
    bool m_reloadPending;                         // 0x1EC
};

#endif
