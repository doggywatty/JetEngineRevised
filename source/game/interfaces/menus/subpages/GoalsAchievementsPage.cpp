// source/game/interfaces/menus/subpages/GoalsAchievementsPage.cpp
#include "GoalsAchievementsPage.h"
#include "game/entities/Achievement.h"
#include "game/entities/AchievementSet.h"
#include "game/interfaces/interfaceobjects/InterfaceGrid.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/box/AchievementsBox.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"

// @0x49e1d4: the size of the page is the one of "Achievements_Size"; the mark of the missions of the layer is hidden
GoalsAchievementsPage::GoalsAchievementsPage() : BasicPage(jet::String("Menu_Missions")), m_achievementsGrid(0) {
    InterfaceObject::SetSize(GetUIObject(jet::String("Achievements_Size"))->GetSize());
    GetUIObject(jet::String("Mission_state"))->SetVisible(false);
    m_achievementsGrid = GetUIGrid(jet::String("Achievements_WorldGrid"));
}

// @0x49e414
GoalsAchievementsPage::~GoalsAchievementsPage() { ClearAchievementArray(); }

// @0x49e49c: the page takes the height of the list it is in; the grid and its scroll bar fill what is left of it
void GoalsAchievementsPage::Refresh() {
    InterfaceObject *parent = m_parent;
    if (parent) {
        const math::vec2<float> parentSize = parent->GetSize();
        if (parentSize.y > GetSize().y) SetSize(parentSize);
        const math::vec2<float> gridPos = m_achievementsGrid->GetPos();
        const math::vec2<float> size = GetSize();
        m_achievementsGrid->SetSize(math::vec2<float>(size.x - gridPos.x, size.y - gridPos.y));
        if (m_achievementsGrid->HasClip()) {
            const math::vec2<float> gridSize = m_achievementsGrid->GetSize();
            m_achievementsGrid->SetClip(math::rect<float>(gridPos.x, gridPos.y, gridPos.x + gridSize.x, gridPos.y + gridSize.y));
        }
        // the scroll bar is as high as the grid, less three points above and below
        InterfaceScrollBar *scrollBar = GetUIScrollBar(jet::String("Achievements_InterfaceScrollBarWorld"));
        const float x = scrollBar->GetUnscaledPos().x;
        const float y = m_achievementsGrid->GetUnscaledPos().y + 3.0f;
        math::vec2<float> barSize = scrollBar->GetUnscaledSize();
        barSize.y = m_achievementsGrid->GetUnscaledSize().y - 6.0f;
        scrollBar->SetPos(math::vec2<float>(x, y));
        scrollBar->SetSize(barSize);
    }
    PopulateAchievementArray();
}

// @0x49e70c
void GoalsAchievementsPage::ClearAchievementArray() {
    for (size_t i = 0; i < m_achievementArray.size(); i++) delete m_achievementArray[i];
    m_achievementArray.clear();
}

// @0x49e754: a box for each achievement the player can see, in the grid, and the text with how many are done
void GoalsAchievementsPage::PopulateAchievementArray() {
    ClearAchievementArray();
    if (!AchievementSet::HasInstance()) return;
    int completed = 0;
    const std::vector<Achievement*> list = AchievementSet::GetInstance()->GetAchievements();
    for (size_t i = 0; i < list.size(); i++) {
        Achievement *a = list[i];
        if (!a->CanBeSeenByUser()) continue;
        if (a->IsCompleted()) completed++;
        AchievementsBox *box = new AchievementsBox(a);
        m_achievementArray.push_back(box);
        box->SetParent(m_achievementsGrid);
    }
    m_achievementsGrid->SetRows((int)m_achievementArray.size());
    GetUIText(jet::String("Achievements_Completed_Text"))
        ->SetText(jet::String::Format(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("CHALLENGE_10")).c_str(), completed, (int)m_achievementArray.size()));
}

// @0x49e9b8: whether the grid is being dragged
bool GoalsAchievementsPage::ShouldResetChildren() const { return m_achievementsGrid->IsDragging(); }
