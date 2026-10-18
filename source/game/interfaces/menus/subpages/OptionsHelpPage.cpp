// source/game/interfaces/menus/subpages/OptionsHelpPage.cpp
#include "OptionsHelpPage.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "game/Game.h"
#include "game/gamemanagers/COPPAMgr.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/utils/GameUtils.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"
#include <cmath>

// @0x3bdf78 (static init)
const jet::String OptionsHelpPage::k_keyTitle("HELP_TITLE_01");
const jet::String OptionsHelpPage::k_keyDescription("HELP_DESCRIPTION_01");

// k_minSwipeX / k_minSwipeY (@0xc89db0)
static const float k_minSwipe = 25.0f;

// @0x4a38b0
OptionsHelpPage::OptionsHelpPage()
    : BasicPage(jet::String("Options_Help")), m_currentElement(1), m_previousButton(0), m_nextButton(0), m_pageText(0), m_currentPage(1), m_titleText(0),
      m_contentsText(0), m_contentsArea(0, 0, 0, 0), m_lastTouch(0, 0) {
    m_totalElements = (unsigned)GameUtils::ElementsCount(k_keyTitle);
    InterfaceObject::SetSize(GetUIObject(jet::String("Size"))->GetSize());
    m_previousButton = GetUIButton(jet::String("Previous_button"));
    m_nextButton = GetUIButton(jet::String("Next_button"));
    m_titleText = GetUIText(jet::String("Title_Text"));
    m_contentsText = GetUIText(jet::String("Contents_Text"));
    m_pageText = GetUIText(jet::String("Pages_Text"));
    m_totalPages = m_totalElements;
    m_keyTitle = k_keyTitle;
    m_keyDescription = k_keyDescription;
    SetText();
}

OptionsHelpPage::~OptionsHelpPage() {}

// @0x4a3d78: the arrows, then a swipe
void OptionsHelpPage::Update(int dt) {
    BasicPage::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    int n = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < n; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_previousButton) {
            PreviousPage();
            break;
        }
        if (b == m_nextButton) {
            NextPage();
            break;
        }
    }
    UpdateBySwipe();
}

// @0x4a3e04
void OptionsHelpPage::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (IsVisible()) BasicPage::Render(cam, p);
}

// @0x4a3e3c: the pages left out are counted again, the page shown is moved off a left out one
void OptionsHelpPage::OnEnter() {
    m_disabledPages.clear();
    DisablePages();
    SetFirstPage();
    m_totalPages = m_totalElements - (unsigned)m_disabledPages.size();
}

// @0x4a3e8c
void OptionsHelpPage::Refresh() {
    InterfaceObject *size = GetUIObject(jet::String("Size"));
    math::vec2<float> off = size->GetOffset(), off2 = size->GetOffset(), sz = size->GetSize();
    m_contentsArea = math::rect<float>(off.x, off.y, sz.x + off2.x, sz.y + off2.y);
    HandleArrows();
    SetText();
}

// @0x4a3f30: the title, the text and "page / pages"
void OptionsHelpPage::SetText() {
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    m_titleText->SetText(sm->Get(m_keyTitle));
    m_contentsText->SetText(COPPAMgr::FixEURSymbol(sm->Get(m_keyDescription)));
    babel::Formatter *f = babel::Babel::GetInstance()->GetFormatter();
    m_pageText->SetText(f->FormatMeasure((int)m_currentPage) + jet::String(" / ") + f->FormatMeasure((int)m_totalPages));
}

// @0x4a408c
void OptionsHelpPage::HandleArrows() {
    m_previousButton->SetVisible(m_currentPage != 1);
    m_nextButton->SetVisible(m_currentPage != m_totalPages);
}

// @0x4a40d4: the keys before; a left out page is passed over. In Italy page 21 has its own text.
void OptionsHelpPage::PreviousPage() {
    if (m_currentPage <= 1) return;
    if (m_keyDescription.Equals("HELP_DESCRIPTION_21_IT") && COPPAMgr::GetInstance()->IsItaliaCountry()) m_keyDescription = "HELP_DESCRIPTION_21";
    jet::String title = GameUtils::IncreaseKey(m_keyTitle, -1), desc = GameUtils::IncreaseKey(m_keyDescription, -1);
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    if (!sm->Exists(title) || !sm->Exists(desc)) return;
    m_keyTitle = title;
    m_keyDescription = desc;
    m_currentElement--;
    if (m_disabledPages.find(m_keyTitle) != m_disabledPages.end()) {
        PreviousPage();
        return;
    }
    m_currentPage--;
    SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
    Refresh();
}

// @0x4a4260
void OptionsHelpPage::NextPage() {
    if (m_currentPage >= m_totalPages) return;
    jet::String title = GameUtils::IncreaseKey(m_keyTitle, 1), desc = GameUtils::IncreaseKey(m_keyDescription, 1);
    if (desc.Equals("HELP_DESCRIPTION_21") && COPPAMgr::GetInstance()->IsItaliaCountry()) desc = "HELP_DESCRIPTION_21_IT";
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    if (!sm->Exists(title) || !sm->Exists(desc)) return;
    m_keyTitle = title;
    m_keyDescription = desc;
    m_currentElement++;
    if (m_disabledPages.find(m_keyTitle) != m_disabledPages.end()) {
        NextPage();
        return;
    }
    m_currentPage++;
    SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
    Refresh();
}

// @0x4a43f0: off the left out pages
void OptionsHelpPage::SetFirstPage() {
    if (m_disabledPages.find(m_keyTitle) == m_disabledPages.end()) return;
    m_keyTitle = GameUtils::IncreaseKey(m_keyTitle, 1);
    m_keyDescription = GameUtils::IncreaseKey(m_keyDescription, 1);
    m_currentElement++;
    SetFirstPage();
}

// @0x4a4478: the shop limits and, without the social features, the multiplier; and the other platforms' pages
void OptionsHelpPage::DisablePages() {
    DisablePage(jet::String("HELP_TITLE_20"));
    if (!Game::GetInstance()->AreSocialFeaturesEnabled()) DisablePage(jet::String("HELP_TITLE_08"));
    DisablePage(jet::String("HELP_TITLE_22"));
    DisablePage(jet::String("HELP_TITLE_04"));
    DisablePage(jet::String("HELP_TITLE_18"));
    DisablePage(jet::String("HELP_TITLE_22"));
    DisablePage(jet::String("HELP_TITLE_23"));
    DisablePage(jet::String("HELP_TITLE_09"));
    DisablePage(jet::String("HELP_TITLE_10"));
}

// @0x4a4668
void OptionsHelpPage::DisablePage(jet::String key) { m_disabledPages.insert(key); }

// @0x4a4680: a touch on the text is remembered when it starts; released more than 25 pixels away along its main
// direction it turns the page (left or up for the next one)
void OptionsHelpPage::UpdateBySwipe() {
    const Touch *t = TouchMgr::GetInstance()->FindTouch(m_contentsArea, -1);
    if (!t) return;
    if (t->state & Touch::Pressed) {
        m_lastTouch = t->pos;
        return;
    }
    if (!(t->state & Touch::Released)) return;
    float dx = m_lastTouch.x - t->pos.x, dy = m_lastTouch.y - t->pos.y;
    float d;
    if (std::fabs(dx) > std::fabs(dy)) {
        if (std::fabs(dx) <= k_minSwipe) return;
        d = dx;
    } else {
        if (std::fabs(dy) <= k_minSwipe) return;
        d = dy;
    }
    if (d > 0) NextPage();
    else PreviousPage();
}
