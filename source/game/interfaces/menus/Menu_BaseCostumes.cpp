// source/game/interfaces/menus/Menu_BaseCostumes.cpp
#include "Menu_BaseCostumes.h"
#include "sound/soundmgr.h"
#include "MenuMgr.h"
#include "camera/cameramgr.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/Player.h"
#include "game/bodypartdetection/BodyPartOwner.h"
#include "game/entities/Deco3d.h"
#include "game/entities/LevelDef.h"
#include "game/entities/LightDef.h"
#include "game/entities/Minion.h"
#include "game/entities/MinionCostume.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamestates/GS_MainMenu.h"
#include "game/GFXConfig.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/CostumeShopItem.h"
#include "game/interfaces/interfaceobjects/InterfaceArea.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/online/store/Price.h"
#include "game/utils/GameUtils.h"
#include "scene/camera.h"
#include "scene/model.h"
#include "scene/scenemgr.h"
#include "game/dynamicobjects/EffectMgr.h"
#include "game/dynamicobjects/ModelPool.h"
#include "sprites/spriteplayer.h"
#include "video/driver.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
const float k_eps = 1.1920929e-07f;
}

const int Menu_BaseCostumes::k_minTouchMoveToSwipe = 20;    // @0xc8a5ac
const int Menu_BaseCostumes::k_maxTouchTimeToSwipe = 600;   // @0xc8a5b0
const jet::String Menu_BaseCostumes::k_sfx_costumes_scroll("ev_sfx_scroll_costumes");  // @0xc3ca3f

// @0x4dfe30
Menu_BaseCostumes::Menu_BaseCostumes(const jet::String &guiLevelName)
    : Menu_Base(guiLevelName), m_currentCostume(-1), m_draggingGrid(false), m_draggingIndex(false), m_currentWasBought(false), m_bigPos(0, 0, 0),
      m_bigSpacing(0), m_currentPos(0), m_item(0), m_scrollBar(0), m_selectedGraph(0), m_gridRect(0, 0, 0, 0), m_indexRect(0, 0, 0, 0), m_bigWorldPerPixel(0),
      m_locked(false), m_effectLayerId(0), m_smallPos(0, 0, 0), m_badgeNew(0), m_badgeTime(0), m_badgePromo(0), m_badgeBought(0), m_badgePuzzle(0),
      m_badgeAward(0), m_badgeAwardSale(0), m_badgeOffset(0, 0, 0), m_smallScreenX(0), m_scrollOffset(0), m_smallSpacing(0), m_halfSmallSpacing(0),
      m_smallWorldPerPixel(0), m_touchTime(0), m_transitionTimer(0), m_costumeGUIVisible(true), m_touchedBPO(0), m_3dTouchesDisableTimer(0),
      m_reloadPending(false) {}

// @0x4dff08
Menu_BaseCostumes::~Menu_BaseCostumes() { ClearCostumes(); }

// @0x4dff98: the rectangles of the two grids, the panel of the selected costume and the scroll bar, which show only
// when the wheel draws them
void Menu_BaseCostumes::Init(const jet::String &grid, const jet::String &indexGrid, const jet::String &selected, const jet::String &scrollBar) {
    m_touchedBPO = 0;
    InterfaceObject *g = GetUIObject(grid);
    math::vec2<float> p = g->GetPos(), s = g->GetSize();
    m_gridRect = math::rect<float>(p.x, p.y, s.x + p.x, s.y + p.y);
    InterfaceObject *i = GetUIObject(indexGrid);
    p = i->GetPos();
    s = i->GetSize();
    m_indexRect = math::rect<float>(p.x, p.y, s.x + p.x, s.y + p.y);
    m_selectedGraph = GetUIGraph(selected);
    m_selectedGraph->SetVisible(false);
    m_scrollBar = (InterfaceScrollBar*)GetUIObject(scrollBar);
    m_scrollBar->SetHorizontal(true);
    m_scrollBar->SetVisible(false);
}

// @0x4e00fc
void Menu_BaseCostumes::ClearCostumes() {
    for (size_t i = 0; i < m_costumes.size(); i++) {
        m_costumes[i]->SetEnableReachTarget(false);
        delete m_costumes[i];
    }
    m_costumes.clear();
    delete m_item;
    m_item = 0;
    for (size_t i = 0; i < m_smallCostumes.size(); i++) delete m_smallCostumes[i];
    m_smallCostumes.clear();
    SpritePlayer **badges[] = { &m_badgeNew, &m_badgeTime, &m_badgePromo, &m_badgeBought, &m_badgeAward, &m_badgeAwardSale, &m_badgePuzzle };
    for (SpritePlayer **b : badges) {
        delete *b;
        *b = 0;
    }
    m_touchedBPO = 0;
}

// @0x4e02a4: the current one goes back to its idle label
void Menu_BaseCostumes::RefreshCostumes() {
    if ((unsigned)m_currentCostume >= m_costumes.size()) return;
    m_costumes[(size_t)m_currentCostume]->SM_SetStateLabel(1);
}

// @0x482608
MinionCostume *Menu_BaseCostumes::GetCurrentCostume() const {
    return (unsigned)m_currentCostume < m_costumes.size() ? m_costumes[(size_t)m_currentCostume] : 0;
}

// @0x4e02cc: the costumes walk to their places (their speed depends on how far they have to go), those near the middle
// are updated, the badges, the panel and the touches follow. b: the costumes of the main menu are in the level's lists;
// c: the menu is leaving (nothing is reloaded and the touches are not looked at)
void Menu_BaseCostumes::UpdateCostumes(int dt, bool b, bool c) {
    if (m_reloadPending && !c) ReloadCostumes();
    if (getenv("JET_DBGCOS")) { static int n = 0; if (n++ % 120 == 0) { math::vec3<float> cp = CameraMgr::GetInstance()->GetSceneCamera()->GetPosition(); fprintf(stderr, "[cos] upd cam=(%f %f %f) reload=%d\n", cp.x, cp.y, cp.z, (int)m_reloadPending); } }
    if (m_3dTouchesDisableTimer > 0) m_3dTouchesDisableTimer -= dt;
    EffectMgr::GetInstance()->Update(m_effectLayerId, dt);  // @0x4e035c
    float bigRange = m_bigSpacing;
    if (std::fabs(m_item->GetOffsetDelayerValue()) <= k_eps) bigRange *= 0.5f;
    const float smallRange = (b ? 3.8f : 2.8f) * m_smallSpacing;
    if (b) {
        for (size_t i = 0; i < m_smallCostumes.size(); i++) {
            MinionCostume *s = m_smallCostumes[i];
            if (s->HasReachedTargetPos()) continue;
            float spacing = m_smallSpacing;
            const float y = s->GetPosition().y;
            if (y < s->GetTargetPosition().y) {
                if (i != 0) spacing = std::max(spacing, std::fabs(y - m_smallCostumes[i - 1]->GetPosition().y));
            } else if (i + 1 < m_smallCostumes.size()) {
                spacing = std::max(spacing, std::fabs(y - m_smallCostumes[i + 1]->GetPosition().y));
            }
            s->SetMoveSpeed(spacing * MinionCostume::k_defaultMoveSpeed / m_smallSpacing);
        }
    }
    for (size_t i = 0; i < m_costumes.size(); i++) {
        MinionCostume *big = m_costumes[i];
        if (big->IsNearTheCurrentPos()) SetCurrentCostume((int)i);
        const bool near = bigRange > std::fabs(big->GetPosition().y - m_bigPos.y);
        if (b) {
            big->GetDeco()->RegisterForUpdate(near);
            big->GetDeco()->RegisterForRender(near);
            if (big->GetShadowDeco()) big->GetShadowDeco()->RegisterForRender(near);
        } else {
            big->Update();
            if (near) big->GetDeco()->Update();
        }
        MinionCostume *small = m_smallCostumes[i];
        small->Update();
        if (smallRange >= std::fabs(small->GetPosition().y - m_smallPos.y)) small->GetDeco()->Update();
    }
    CurrentCostume_CheckPlaySpecificAnim();
    SpritePlayer *badges[] = { m_badgeNew, m_badgeTime, m_badgePromo, m_badgeBought, m_badgeAward, m_badgeAwardSale, m_badgePuzzle };
    for (SpritePlayer *s : badges) s->Update(dt);
    if (m_currentCostume >= 0) m_item->Update(dt);
    if (!c) UpdateTouchesForCostumes(dt, b);
    else MinionCostume::SetIsDragging(false);
}

// @0x4e06d0: a finger on the wheel scrolls it (the big costumes' grid by their distance on screen, the strip by its own);
// when it lets go the wheel snaps to the nearest costume, or the next one when it was a quick swipe; a tap on the strip
// goes there. The scroll bar shows how far the wheel is, the panel is on while it stands still, and when it does the
// touches go to the 3D costume.
void Menu_BaseCostumes::UpdateTouchesForCostumes(int dt, bool c) {
    TouchMgr *tm = TouchMgr::GetInstance();
    const Touch *t = tm->FindTouch(m_gridRect, -1);
    bool inGrid;
    if (m_draggingGrid || m_draggingIndex) {
        if (!t) t = tm->FindTouch(-1);
        inGrid = true;
    } else if (t) {
        inGrid = true;
    } else {
        t = tm->FindTouch(m_indexRect, -1);
        inGrid = false;
    }
    if (!m_costumes.empty() && (unsigned)m_currentCostume < m_costumes.size() && !m_locked) {
        const bool touching = t && !(t->state & 0xc);
        bool fromGrid;
        if (m_draggingGrid || (!m_draggingIndex && inGrid)) {
            m_draggingGrid = touching;
            fromGrid = true;
        } else {
            m_draggingIndex = touching;
            fromGrid = false;
        }
        float dx = 0.0f;
        if (!touching) SetItemsButtonsActive(true);
        else dx = t->pos.x - t->prevPos.x;
        MinionCostume::SetIsDragging(touching);
        if (t) m_touchTime += dt;
        if (!m_costumes[(size_t)m_currentCostume]->HasReachedTargetPos()) {
            m_transitionTimer += dt;
        } else {
            if (m_transitionTimer > 0) {
                if ((float)m_transitionTimer > 500.0f) m_transitionTimer = 500;
                m_transitionTimer -= dt;
            }
            m_item->SetTransition(m_costumeGUIVisible);
        }
        if (t && t->state == Touch::Pressed) {
            m_touchTime = 0;
        } else if (std::fabs(dx) > k_eps || !touching) {
            const float scroll = fromGrid ? m_smallSpacing * m_bigWorldPerPixel * dx / m_bigSpacing : dx * m_smallWorldPerPixel;
            float offset = m_scrollOffset;
            if (touching) {
                m_scrollOffset = offset - scroll;
            } else if (std::fabs(offset + m_smallPos.y - m_currentPos) > m_halfSmallSpacing) {
                SoundMgr::GetInstance()->Play(Menu_BaseCostumes::k_sfx_costumes_scroll, 0);  // @0x4e0a20
            } else {
                const float swipe = t ? t->pos.x - t->startPos.x : 0.0f;
                if (std::fabs(swipe) > 20.0f) {
                    if (m_touchTime <= 600) {
                        const float spacing = m_smallSpacing;
                        offset -= (float)std::fmod((double)offset, (double)spacing);
                        if (swipe <= 0) offset += spacing;
                        m_scrollOffset = offset;
                        SoundMgr::GetInstance()->Play(Menu_BaseCostumes::k_sfx_costumes_scroll, 0);
                    }
                } else if (t && !fromGrid) {
                    m_scrollOffset = offset + (t->pos.x - m_smallScreenX) * m_smallWorldPerPixel;
                    CheckCostumesScrollBoundaries(m_scrollOffset);
                    // the sound plays when the tap is more than half a step away (the branch above)
                }
            }
            if (t) ApplyOffset(touching, c);
        }
    }
    if (m_smallCostumes.size() >= 2) {
        const float span = m_smallCostumes.front()->GetPosition().y - m_smallCostumes.back()->GetPosition().y;
        m_scrollBar->SetValue((m_smallCostumes.front()->GetPosition().y - m_smallPos.y) / span);
    }
    const float opacity = std::min(std::max((float)m_transitionTimer * 0.002f, 0.0f), 1.0f);
    m_scrollBar->SetOpacity(opacity);
    m_selectedGraph->SetColor(((unsigned)((1.0f - opacity) * 255.0f) << 24) | 0xffffffu);
    if (IsScrollingCostumes()) {
        m_item->Expand(true, false);
        m_item->SetTransition(false);
    } else {
        Update3DTouches(tm->FindTouch(-1));
    }
}

// @0x4e0c00: the wheel stays between the first and the last costume
bool Menu_BaseCostumes::CheckCostumesScrollBoundaries(float &v) {
    if (v < 0) {
        v = 0;
        return true;
    }
    const float max = (float)(unsigned)(m_smallCostumes.size() - 1) * m_smallSpacing;
    if (v > max) {
        v = max;
        return true;
    }
    return false;
}

// @0x4e0c5c: the panel of the selected costume and the scroll bar, then the costumes that are near enough to the middle
// in the world's scene with the menu's light (the main menu's big ones are drawn by the level itself), and the badges
// that tell what is new, on sale, a puzzle or awarded over the small ones
void Menu_BaseCostumes::RenderCostumes(const jet::scene::Camera &cam, jet::video::Painter &painter, bool smallOnly) {
    m_selectedGraph->SetVisible(true);
    m_selectedGraph->Render(cam, painter);
    m_selectedGraph->SetVisible(false);
    m_scrollBar->SetVisible(true);
    m_scrollBar->Render(cam, painter);
    m_scrollBar->SetVisible(false);
    jet::video::Driver *driver = jet::video::Driver::GetCurrent();
    driver->Flush();
    jet::scene::SceneMgr *sm = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World);
    jet::scene::Camera *wc = CameraMgr::GetInstance()->GetSceneCamera();
    sm->SetCamera(wc);
    MenuMgr *mm = MenuMgr::GetInstance();
    if (LightDef *light = mm->GetLight()) {
        light->AttachToCamera(wc, light->GetCameraOffset());
        driver->ResetLights();
        driver->AddLight(light->GetLightData()->data);
    }
    float bigRange = m_bigSpacing;
    if (std::fabs(m_item->GetOffsetDelayerValue()) <= k_eps) bigRange *= 0.5f;
    if (!smallOnly) {
        for (MinionCostume *c : m_costumes) {
            if (bigRange > std::fabs(c->GetPosition().y - m_bigPos.y)) {
                c->GetDeco()->Render();
                if (c->GetShadowDeco()) c->GetShadowDeco()->Render();
            }
        }
    }
    const float smallRange = (smallOnly ? 3.8f : 2.8f) * m_smallSpacing;
    for (MinionCostume *c : m_smallCostumes) {
        if (smallRange > std::fabs(c->GetPosition().y - m_smallPos.y)) {
            c->GetDeco()->Render();
            if (c->GetShadowDeco()) c->GetShadowDeco()->Render();
        }
    }
    mm->SetCameraDataForParticles(*wc);
    ModelPool::GetInstance()->Render(sm, m_effectLayerId);  // @0x4e0f10
    GameUtils::MenuRenderScene(sm, 0, false, false);
    EffectMgr::GetInstance()->Render(m_effectLayerId);  // @0x4e0f38
    mm->SetCameraDataForParticles(*mm->GetCamera());
    driver->Flush();
    Player *pl = Player::GetInstance();
    for (MinionCostume *c : m_smallCostumes) {
        if (!(smallRange > std::fabs(c->GetPosition().y - m_smallPos.y))) continue;
        const bool bought = CostumeMgr::GetInstance()->WasBought(*c);
        const bool puzzle = c->GetBuyType().Equals(MinionCostume::k_buyType_puzzle);
        const int first = pl->IsGetFirstCostume(), second = pl->IsGetSecondCostume(), magic = pl->IsMigicCostume();
        math::vec3<float> screen(0, 0, 0);
        const math::vec3<float> at = c->GetPosition() + m_badgeOffset;
        wc->Project(screen, at);
        const int id = c->GetCostumeId();
        SpritePlayer *badge = 0;
        if (puzzle || bought || c->HasPromo()) {
            const bool sale = c->GetPrice()->HasPricePromotion();
            const bool limited = c->IsTimeLimited();
            if (bought) {
                badge = m_badgeBought;
                if (id == 0x12) {
                    if (first == 1) badge = m_badgeAward;
                    else if (first == 2) badge = m_badgeAwardSale;
                } else if (id == 0x10) {
                    if (second == 1) badge = m_badgeAward;
                    else if (second == 2) badge = m_badgeAwardSale;
                } else if (id == 0x19) {
                    if (magic == 2) badge = m_badgeAwardSale;
                }
            } else if (puzzle) {
                badge = m_badgePuzzle;
            } else if (sale) {
                badge = m_badgePromo;
            } else {
                badge = limited ? m_badgeTime : m_badgeNew;
            }
        } else if ((id == 0x12 && first == 2) || (id == 0x10 && second == 2) || (id == 0x19 && magic == 2)) {
            badge = m_badgeAwardSale;
        }
        if (badge) {
            badge->SetPosition(screen);
            badge->Render(painter);
        }
    }
    if (m_currentCostume >= 0 && !m_costumes.empty()) m_item->Render(cam, painter);
}

// @0x4e11f4: where the costumes stand: the big one's feet are the grid's element projected into the 3D scene at the
// distance the level says; the distance to the next one's gives their spacing, and the same for the small ones
void Menu_BaseCostumes::InitCostumes(const jet::String &bigFeet, const jet::String &bigNextFeet, const jet::String &smallFeet, const jet::String &smallNextFeet) {
    const bool inMainMenu = GameState::GetCrtState()->GetStateName().Equals(GS_MainMenu::k_stateName);
    m_currentCostume = -1;
    jet::scene::Camera *cam = CameraMgr::GetInstance()->GetSceneCamera();
    const math::vec3<float> camPos = cam->GetPosition();
    Game *game = Game::GetInstance();
    float aspect = 1.0f;
    if (!inMainMenu) aspect = game->GetDesignWidthScale() / (float)game->GetGFXConfig()->designWidth * 0.75f;
    math::vec3<float> front = cam->GetFrontVector();
    float dist = LevelDef::GetInstance()->GetI3DShopBigMinionCostumeCamDistance();
    math::vec3<float> p = camPos + math::vec3<float>(dist * front.x, dist * front.y, dist * front.z);
    math::vec3<float> screen(0, 0, 0);
    cam->Project(screen, p);
    math::vec2<float> pos = GetUIArea(bigFeet)->GetPos();
    math::vec3<float> q(pos.x, pos.y, screen.z);
    if (inMainMenu) m_bigPos = GameLevel::GetInstance()->GetMinion()->GetInitialPosition();
    else cam->Unproject(m_bigPos, q);
    math::vec3<float> world(0, 0, 0);
    math::vec2<float> next = GetUIArea(bigNextFeet)->GetPos();
    const float dxBig = next.x - q.x;
    q.x = dxBig + q.x;
    cam->Unproject(world, q);
    m_bigSpacing = m_bigPos.y - world.y;
    m_bigWorldPerPixel = m_bigSpacing / dxBig;
    MinionCostume::SetCurrentBigCostumePosWorldY(m_bigPos.y);

    front = cam->GetFrontVector();
    dist = LevelDef::GetInstance()->GetI3DShopSmallMinionCostumeCamDistance();
    if (getenv("JET_DBGCOS")) fprintf(stderr, "[cos] smalldist=%f bigscr=(%f %f %f) front=(%f %f %f) cam=(%f %f %f) aspect=%f\n", dist, screen.x, screen.y, screen.z, front.x, front.y, front.z, camPos.x, camPos.y, camPos.z, aspect);
    p = camPos + math::vec3<float>(aspect * front.x * dist, aspect * front.y * dist, aspect * front.z * dist);
    cam->Project(screen, p);
    pos = GetUIArea(smallFeet)->GetPos();
    m_smallScreenX = pos.x;
    q = math::vec3<float>(pos.x, pos.y, screen.z);
    cam->Unproject(m_smallPos, q);
    next = GetUIArea(smallNextFeet)->GetPos();
    const float dxSmall = next.x - q.x;
    q.x = dxSmall + q.x;
    cam->Unproject(world, q);
    m_smallSpacing = m_smallPos.y - world.y;
    m_halfSmallSpacing = m_smallSpacing * 0.5f;
    m_smallWorldPerPixel = m_smallSpacing / dxSmall;
    ReloadCostumes();
}

// @0x4e1568: the costumes the shop shows (the ones that load, the first few in the order of the data, the rest after,
// placeholders for what the player owns and the data does not have, a question mark for the ones that cannot load),
// each as a big copy and a small one at their place; the wheel stops at the equipped one, or the one that was selected
void Menu_BaseCostumes::ReloadCostumes() {
    m_reloadPending = false;
    const bool inMainMenu = GameState::GetCrtState()->GetStateName().Equals(GS_MainMenu::k_stateName);
    const unsigned ctorFlags = inMainMenu ? 0u : 2u;
    CostumeMgr *cm = CostumeMgr::GetInstance();
    const unsigned total = cm->GetCostumeRefCount();
    if (getenv("JET_DBGCOS")) for (unsigned i = 0; i < total; i++) fprintf(stderr, "[cos] ref %u ord=%d id=%d %s show=%d\n", i, cm->GetCostumeRef(i)->GetOrderValue(), cm->GetCostumeRef(i)->GetCostumeId(), cm->GetCostumeRef(i)->GetClaraEntityName().c_str(), (int)cm->GetCostumeRef(i)->ShouldBeShownInShop());
    static const unsigned k_first[8] = { 0xf, 6, 0xc, 0xb, 1, 2, 0x15, 0 };
    std::vector<MinionCostume*> shown;
    bool promoOrPuzzle = false;
    unsigned unavailable = 0;
    auto consider = [&](MinionCostume *ref) {
        if (!ref->ShouldBeShownInShop()) return;
        if (ref->CanLoadModel()) {
            shown.push_back(ref);
            if (!promoOrPuzzle) promoOrPuzzle = ref->HasPromo() || ref->GetBuyType().Equals(MinionCostume::k_buyType_puzzle);
        } else {
            fprintf(stderr, "[Jet] Costume cannot load model, name: %s\n", ref->GetClaraEntityName().c_str());
            if (!ref->GetClaraEntityName().Equals("16_MinionCostume_Jelly")) unavailable++;
        }
    };
    for (unsigned i = 0; i < 8; i++)
        if (k_first[i] < total) consider(cm->GetCostumeRef(k_first[i]));
    for (unsigned i = 0; i < total; i++) {
        bool first = false;
        for (unsigned f : k_first) first |= f == i;
        if (!first) consider(cm->GetCostumeRef(i));
    }
    const unsigned notInData = cm->GetCountBoughtCostumesNotAvailableInData();
    for (unsigned i = 0; i < notInData; i++) shown.push_back(cm->GetPlaceHolderCostumeRef());
    if (unavailable) {
        fprintf(stderr, "[Jet] Add question mark!\n");
        shown.push_back(cm->GetPlaceHolderCostumeRef());
    }
    // the costume the wheel was at stays selected
    jet::String selected;
    if ((unsigned)m_currentCostume < m_costumes.size()) selected = m_costumes[(size_t)m_currentCostume]->GetClaraEntityName();
    ClearCostumes();
    GameLevel::GetInstance()->RemoveEntities();
    int keep = -1;
    m_currentCostume = -1;
    for (size_t i = 0; i < shown.size(); i++) {
        if (shown[i]->GetClaraEntityName().Equals(selected)) keep = (int)i;
        m_costumes.push_back(new MinionCostume(shown[i], ctorFlags));
    }
    if (std::fabs(m_currentPos) <= k_eps) {
        m_currentPos = m_smallPos.y;
        for (size_t i = 0; i < m_costumes.size(); i++)
            if (m_costumes[i]->IsCurrentlyEquipped()) {
                m_currentPos += (float)i * m_smallSpacing;
                break;
            }
    }
    if (keep >= 0) m_currentPos = m_smallPos.y + (float)keep * m_smallSpacing;
    const float offset = m_currentPos - m_smallPos.y;
    m_scrollOffset = offset;
    const float bigY = offset * m_bigSpacing / m_smallSpacing + m_bigPos.y;
    const float half = MinionCostume::k_baseAngle * 0.5f;
    const math::quat<float> facing(0, 0, std::sin(half), std::cos(half));
    m_item = new CostumeShopItem(inMainMenu);
    m_effectLayerId = EffectMgr::GetNextLayerId();
    const float far = m_smallSpacing * 2.3f;
    for (size_t i = 0; i < m_costumes.size(); i++) {
        MinionCostume *big = m_costumes[i];
        const math::vec3<float> bigAt(m_bigPos.x, bigY - (float)i * m_bigSpacing, m_bigPos.z);
        big->SetPosition(bigAt);
        big->SetTargetPosition(bigAt);
        big->SetEnableReachTarget(true);
        big->SetTracksEnabled(false, false);
        big->GetDeco()->SetRotation(facing);
        big->GetDeco()->SetRotationToReach(facing, 0);
        if (!inMainMenu) big->SetEffectLayerId(m_effectLayerId);
        if (big->IsNearTheCurrentPos()) SetCurrentCostume((int)i);
        MinionCostume *small = new MinionCostume(big, 2);
        if (!big->CanLoadModel()) {
            big->SM_SetAnimatedCostume(0);
            small->SM_SetAnimatedCostume(0);
        }
        float y = m_currentPos - (float)i * m_smallSpacing;
        if (inMainMenu) {
            float delta = y - m_smallPos.y;
            if (far < std::fabs(delta)) y += (delta <= 0 ? -far : far) * 0.5f;
        }
        const math::vec3<float> smallAt(m_smallPos.x, y, m_smallPos.z);
        small->SetPosition(smallAt);
        small->SetTargetPosition(smallAt);
        small->SetEnableReachTarget(true);
        small->SetTracksEnabled(false, false);
        small->GetDeco()->SetRotation(facing);
        small->GetDeco()->SetRotationToReach(facing, 0);
        small->SetEffectLayerId(m_effectLayerId);
        m_smallCostumes.push_back(small);
        // the badges go to the left edge of the last one's model (every costume's is as wide)
        if (i == m_costumes.size() - 1 && std::fabs(m_badgeOffset.x) <= k_eps && std::fabs(m_badgeOffset.y) <= k_eps && std::fabs(m_badgeOffset.z) <= k_eps) {
            small->GetDeco()->Update();
            const math::aabb<float> box = small->GetDeco()->GetModel()->ComputeAccurateBoundingBox();
            m_badgeOffset = math::vec3<float>(0, -box.maxPoint.x, 0);
        }
        // the prices of the costumes that are awarded for a while are 20% off
        Price *price = big->GetPrice();
        if (price->GetPriceType() != Price::IAP) {
            price->SetPrice((double)(int)price->GetRegularPriceAsDouble());
            if (big->GetCostumeId() == 0x12 && Player::GetInstance()->IsGetFirstCostume() == 2)
                price->SetPrice((double)(int)(price->GetRegularPriceAsDouble() * 0.800000011920929));
            if (big->GetCostumeId() == 0x10 && Player::GetInstance()->IsGetSecondCostume() == 2)
                price->SetPrice((double)(int)(price->GetRegularPriceAsDouble() * 0.800000011920929));
        }
    }
    if (getenv("JET_DBGCOS")) {
        fprintf(stderr, "[cos] n=%zu big=(%f %f %f) sp=%f small=(%f %f %f) sp=%f cur=%d\n", m_costumes.size(), m_bigPos.x, m_bigPos.y, m_bigPos.z, m_bigSpacing, m_smallPos.x, m_smallPos.y, m_smallPos.z, m_smallSpacing, m_currentCostume);
        for (size_t i = 0; i < m_costumes.size() && i < 4; i++) {
            MinionCostume *c = m_costumes[i];
            math::vec3<float> p = c->GetPosition();
            fprintf(stderr, "[cos] shadow=%p %zu %s pos=(%f %f %f) deco=%p model=%p small.deco model=%p\n", (void*)c->GetShadowDeco(), i, c->GetClaraEntityName().c_str(), p.x, p.y, p.z, (void*)c->GetDeco(), c->GetDeco() ? (void*)c->GetDeco()->GetModel() : 0, m_smallCostumes[i]->GetDeco() ? (void*)m_smallCostumes[i]->GetDeco()->GetModel() : 0);
        }
    }
    if (getenv("JET_DBGMAT") && !m_costumes.empty()) {
        jet::scene::Model *md = m_costumes[0]->GetDeco()->GetModel();
        fprintf(stderr, "[mat] model %s materials %u\n", m_costumes[0]->GetDeco()->GetModelName().c_str(), md->GetMaterialCount());
        for (unsigned i = 0; i < md->GetMaterialCount(); i++) {
            const jet::video::Material &m = md->GetMaterial(i);
            fprintf(stderr, "[mat] %u %s tech=%s passes=%u\n", i, md->GetMaterialName(i).c_str(), m.GetRenderTechnique() ? m.GetRenderTechnique()->GetName().c_str() : "-", m.GetRenderPassCount());
            for (unsigned p = 0; p < m.GetRenderPassCount(); p++) {
                for (unsigned t = 0; t < m.GetTextureLayerCount(p); t++) fprintf(stderr, "[mat]   pass %u tex %s\n", p, m.GetTextureLayerName(p, t).c_str());
                for (unsigned u = 0; u < m.GetUniformCount(p); u++) { const unsigned char *d = m.GetUniformData(p, u); fprintf(stderr, "[mat]   pass %u uniform %s %.3f %.3f %.3f %.3f\n", p, m.GetUniformName(p, u).c_str(), ((const float*)d)[0], ((const float*)d)[1], ((const float*)d)[2], ((const float*)d)[3]); }
            }
        }
    }
    static const struct { const char *sprite; int anim; } k_badges[7] = { { "menu_01", 0x91 }, { "menu_01", 0xdf }, { "menu_01", 0xe0 }, { "menu_01", 0xfa },
                                                                          { "menu_01", 0x104 }, { "cdu", 0x1a }, { "menu_01", 0xdf } };
    SpritePlayer **badges[] = { &m_badgeNew, &m_badgeTime, &m_badgePromo, &m_badgeBought, &m_badgePuzzle, &m_badgeAward, &m_badgeAwardSale };
    for (int i = 0; i < 7; i++) {
        *badges[i] = new SpritePlayer(jet::String(k_badges[i].sprite), 0, 0, 0);
        (*badges[i])->SetAnim(k_badges[i].anim, true);
        if (inMainMenu) (*badges[i])->SetScale(math::vec3<float>(0.6f, 0.6f, 0.6f));
    }
}

// @0x4e2208: awarded costumes first, those on sale (a time limited one before the others); then, when asked, the equipped
// one; the puzzles; the costumes bought with tokens before the ones with bananas; the dearer ones, then the order of the data
bool Menu_BaseCostumes::CostumesComparison::operator()(MinionCostume *a, MinionCostume *b) const {
    const bool pa = a->HasPromo(), pb = b->HasPromo();
    if (pa && pb) {
        const bool la = a->IsTimeLimited(), lb = b->IsTimeLimited();
        if (la && !lb) return true;
        if (!la && lb) return false;
    }
    if (pa && !pb) return true;
    if (!pa && pb) return false;
    if (m_equippedFirst) {
        const bool ea = a->IsCurrentlyEquipped(), eb = b->IsCurrentlyEquipped();
        if (ea && !eb) return true;
        if (eb && !ea) return false;
    }
    const bool sa = a->GetBuyType().Equals(MinionCostume::k_buyType_puzzle), sb = b->GetBuyType().Equals(MinionCostume::k_buyType_puzzle);
    if (sa && !sb) return true;
    if (sb && !sa) return false;
    Price *ra = a->GetPrice(), *rb = b->GetPrice();
    if (ra->GetPriceType() == Price::Tokens && rb->GetPriceType() == Price::Bananas) return true;
    if (rb->GetPriceType() == Price::Tokens && ra->GetPriceType() == Price::Bananas) return false;
    if (ra->GetPriceAsDouble() != rb->GetPriceAsDouble()) return ra->GetPriceAsDouble() > rb->GetPriceAsDouble();
    return a->GetOrderValue() < b->GetOrderValue();
}

// @0x4e2414
void Menu_BaseCostumes::SetItemsButtonsActive(bool a) { m_item->SetButtonsActive(a); }

// @0x4e241c: the costume the wheel is at: its panel shows, the one before stops playing its tracks
void Menu_BaseCostumes::SetCurrentCostume(int i) {
    if (m_currentCostume == i || i < 0) return;
    if (m_currentCostume >= 0 && (size_t)m_currentCostume < m_costumes.size()) m_costumes[(size_t)m_currentCostume]->SetTracksEnabled(false, false);
    m_currentCostume = i;
    MinionCostume *c = m_costumes[(size_t)i];
    m_item->SetCostume(c);
    m_currentWasBought = CurrentCostume_WasBought();
    const bool on = !m_item->IsDlcRequired();
    c->SetTracksEnabled(on, on);
}

// @0x4e24b8: the current costume is running to its place
bool Menu_BaseCostumes::IsScrollingCostumes() const {
    if (m_costumes.empty() || m_currentCostume < 0) return false;
    const ActorState *s = m_costumes[(size_t)m_currentCostume]->SM_GetState();
    return s->m_label == 3;
}

// @0x4e24fc: a finger on a body part of the current costume presses it, lifting it there touches it; the one before
// it loses the finger
void Menu_BaseCostumes::Update3DTouches(const Touch *t) {
    if (!Are3DTouchesEnabled()) return;
    if (!t) {
        if (m_touchedBPO) m_touchedBPO->BPO_ClearBodyPartHits();
        m_touchedBPO = 0;
        return;
    }
    const bool released = (t->state & 0xc) || t->state == 0;
    BodyPartOwner *bpo = GetTouchedBPOFor(t->pos);
    if (bpo && bpo->BPO_GetLastHitCount()) {
        BodyPartSection *s = bpo->BPO_GetLastHit(0);
        if (released) bpo->BPO_OnBodyPartTouched(s);
        else bpo->BPO_OnBodyPartPressed(s);
    }
    if (m_touchedBPO && m_touchedBPO != bpo)
        m_touchedBPO->BPO_OnBodyPartReleased(m_touchedBPO->BPO_GetLastHitCount() ? m_touchedBPO->BPO_GetLastHit(0) : 0);
    m_touchedBPO = bpo;
}

// @0x4e25f0: the owner of the body part under a point of the screen: in the menu's scene, else in the level's
BodyPartOwner *Menu_BaseCostumes::GetTouchedBPOFor(const math::vec2<float> &screen) {
    math::vec3<float> hit(0, 0, 0);
    math::quat<float> rot(0, 0, 0, 1);
    jet::scene::Camera *mc = MenuMgr::GetInstance()->GetCamera();
    math::line3<float> ray = mc->GetRayFromScreenCoordinates(screen);
    BodyPartOwner *o = BodyPartOwner::BPO_GetProjectileHit(*mc, ray.start, ray.end, hit, rot, 0, true, true, 8);
    if (getenv("JET_DBGTOUCH")) { fprintf(stderr, "[touch] screen %.0f,%.0f menuhit=%p damageables=%zu\n", screen.x, screen.y, (void*)o, BodyPartOwner::BPO_GetDamageables().size()); for (BodyPartOwner *d : BodyPartOwner::BPO_GetDamageables()) fprintf(stderr, "[touch]   owner %p class %d parts %u\n", (void*)d, (int)d->GetClassType(), d->BPO_GetBodyPartCount()); }
    if (o) return o;
    jet::scene::Camera *wc = CameraMgr::GetInstance()->GetSceneCamera();
    ray = wc->GetRayFromScreenCoordinates(screen);
    return BodyPartOwner::BPO_GetProjectileHit(*wc, ray.start, ray.end, hit, rot, 0, true, true, 7);
}

// @0x4e271c
bool Menu_BaseCostumes::CanScrollCostumes(int n) {
    float v = m_scrollOffset + (float)n * m_smallSpacing;
    return !CheckCostumesScrollBoundaries(v);
}

// @0x4e274c: n costumes along
void Menu_BaseCostumes::ScrollCostumes(int n) {
    m_scrollOffset += (float)n * m_smallSpacing;
    SoundMgr::GetInstance()->Play(Menu_BaseCostumes::k_sfx_costumes_scroll, 0);
    ApplyOffset(false, true);
}

// @0x4e27b0: puts the costumes' targets where the wheel's position says. Let go, the wheel rests on a costume (the
// nearest); the small ones that are far away are pushed further out of the strip
void Menu_BaseCostumes::ApplyOffset(bool touching, bool b) {
    CheckCostumesScrollBoundaries(m_scrollOffset);
    if (!touching) {
        const float spacing = m_smallSpacing;
        float offset = m_scrollOffset;
        const float rest = (float)std::fmod((double)offset, (double)spacing);
        offset -= rest;
        m_scrollOffset = offset;
        if (rest > m_halfSmallSpacing) m_scrollOffset = offset + spacing;
    }
    float smallY = m_scrollOffset + m_smallPos.y;
    float bigY = m_scrollOffset * m_bigSpacing / m_smallSpacing + m_bigPos.y;
    if (!touching) m_currentPos = smallY;
    const float far = m_smallSpacing * 2.3f;
    for (size_t i = 0; i < m_costumes.size(); i++) {
        MinionCostume *big = m_costumes[i], *small = m_smallCostumes[i];
        float y = smallY;
        if (b) {
            const float delta = smallY - m_smallPos.y;
            if (far < std::fabs(delta)) y = (delta <= 0 ? -far : far) * 0.5f + smallY;
        }
        math::vec3<float> st = small->GetTargetPosition();
        st.y = y;
        small->SetTargetPosition(st);
        math::vec3<float> bt = big->GetTargetPosition();
        bt.y = bigY;
        big->SetTargetPosition(bt);
        smallY -= m_smallSpacing;
        bigY -= m_bigSpacing;
    }
}

// @0x4e28f8
bool Menu_BaseCostumes::CurrentCostume_WasBought() const {
    if ((unsigned)m_currentCostume >= m_costumes.size()) return false;
    return CostumeMgr::GetInstance()->WasBought(*m_costumes[(size_t)m_currentCostume]);
}

// @0x4e293c: the current costume plays its happy animation (engine event 0x98) once, when it is known to be bought
void Menu_BaseCostumes::CurrentCostume_CheckPlaySpecificAnim() {
    if ((unsigned)m_currentCostume >= m_costumes.size() || m_currentWasBought) return;
    if (!CurrentCostume_WasBought()) return;
    m_currentWasBought = true;
    m_costumes[(size_t)m_currentCostume]->SM_OnEngineEvent(0x98);
}
