// source/game/interfaces/menus/subpages/GoalsLocationsPage.cpp
#include "GoalsLocationsPage.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/gameenums.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceArea.h"
#include "game/interfaces/interfaceobjects/InterfaceGrid.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include "game/interfaces/menus/box/LocationBox.h"
#include "game/interfaces/menus/box/MissionsMultiplierBox.h"

// @0x49f8bc
GoalsLocationsPage::GoalsLocationsPage() : BasicPage(jet::String("Goals_Locations")), m_missionsMultiplierBox(0), m_grid(0) {
    SetSize(GetUIObject(jet::String("Subpage_Size"))->GetSize());
    m_grid = GetUIGrid(jet::String("Grid"));
}

// @0x49faec
GoalsLocationsPage::~GoalsLocationsPage() {
    ClearLocationBoxData();
    delete m_missionsMultiplierBox;
}

// @0x49fb84: the grid and its clip take the room the page has below their position, the scroll bar fits beside the grid, and the
// box of the multiplier is made again
void GoalsLocationsPage::Refresh() {
    if (InterfaceObject *parent = m_parent) {
        math::vec2<float> parentSize = parent->GetSize(), size = GetSize();
        if (parentSize.y > size.y) SetSize(parentSize);
        math::vec2<float> gridPos = m_grid->GetPos();
        size = GetSize();
        m_grid->SetSize(math::vec2<float>(size.x - gridPos.x, size.y - gridPos.y));
        if (m_grid->HasClip()) {
            math::vec2<float> gridSize = m_grid->GetSize();
            m_grid->SetClip(math::rect<float>(gridPos.x, gridPos.y, gridPos.x + gridSize.x, gridPos.y + gridSize.y));
        }
        InterfaceScrollBar *bar = GetUIScrollBar(jet::String("GridScrollBar"));
        math::vec2<float> barPos = bar->GetUnscaledPos();
        float y = m_grid->GetUnscaledPos().y + 3.0f;
        math::vec2<float> barSize = bar->GetUnscaledSize();
        barSize.y = m_grid->GetUnscaledSize().y - 6.0f;
        bar->SetPos(math::vec2<float>(barPos.x, y));
        bar->SetSize(barSize);
    }
    InitMissionMultiplier();
}

// @0x49ffe8
bool GoalsLocationsPage::ShouldResetChildren() const { return m_grid->IsGridDragging(); }

// @0x49fdf8
void GoalsLocationsPage::ClearLocationBoxData() {
    for (size_t i = 0; i < m_locationBoxes.size(); i++) delete m_locationBoxes[i];
    m_locationBoxes.clear();
}

// @0x49fe48: a box for each location of the data that is not hidden, sorted by the multiplier it asks for, in the grid
void GoalsLocationsPage::PopulateLocationBoxData() {
    ClearLocationBoxData();
    LocationsMgr *lm = LocationsMgr::GetInstance();
    for (int l = 0; l < ELocationDef::COUNT; l++) {
        LocationInfoEntity *info = lm->GetLocationInfo(l);
        if (info && !info->IsHidden()) m_locationBoxes.push_back(new LocationBox(info->GetLocationId()));
    }
    size_t n = m_locationBoxes.size();
    for (size_t i = 0; i + 1 < n; i++)
        for (size_t j = i + 1; j < n; j++)
            if (m_locationBoxes[i]->GetScoreMultiplierValue() > m_locationBoxes[j]->GetScoreMultiplierValue()) std::swap(m_locationBoxes[i], m_locationBoxes[j]);
    for (size_t i = 0; i < n; i++) m_locationBoxes[i]->SetParent(m_grid);
    m_grid->SetRowCount((int)n);
}

// @0x49fff4
void GoalsLocationsPage::InitMissionMultiplier() {
    delete m_missionsMultiplierBox;
    m_missionsMultiplierBox = new MissionsMultiplierBox(false);
    InterfaceArea *offset = GetUIArea(jet::String("mission_nextMultiplierOffset"));
    m_missionsMultiplierBox->SetOffset(offset->GetPos());
    m_missionsMultiplierBox->SetParent(offset);
}
