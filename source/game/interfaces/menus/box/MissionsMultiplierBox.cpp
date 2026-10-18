// source/game/interfaces/menus/box/MissionsMultiplierBox.cpp
//
// Not ported: the particle systems of the bar and of the number (ps::ParticleSystem: EnableParticleSystem, Reset) and the
// debug text the cheats show over the box (CheatsMgr::IsDebugMissionValueActive).
#include "MissionsMultiplierBox.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamemanagers/MissionMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"

// @0x4d6380
MissionsMultiplierBox::MissionsMultiplierBox(bool followsSavedPoints)
    : BasicPage(jet::String("MissionsMultiplierBox")), m_followsSavedPoints(followsSavedPoints), m_pointsToAdd(0), m_popState(0), m_popTimer(0), m_multiplierTextEffectTimer(0),
      m_pointsAddedCount(0) {
    SetSize(GetUIObject(jet::String("BoxSize"))->GetSize());
    m_multiplierPBar = GetUIPBar(jet::String("missions_bar"));
    m_multiplierPBar->SetSmoothValue(true);
    // InterfacePBar::EnableParticleSystem(false)
    m_barStart = GetUIGraph(jet::String("bar_start"));
    m_curMultiplierText = GetUIText(jet::String("multiplier_current"));
    m_nextMultiplierText = GetUIText(jet::String("multiplier_goal"));
    m_multiplierLevel = (int)MissionMgr::GetInstance()->GetMultiplierLevel() + 1;
    m_shownMultiplierLevel = m_multiplierLevel;
    m_multiplierLevelPoints = (int)MissionMgr::GetInstance()->GetMultiplierLevelPoints();
    AddMultiplierPoints(0);
    SetMultiplierText(m_multiplierLevel, 1.0f);
}

// @0x4d6798
MissionsMultiplierBox::~MissionsMultiplierBox() {}

// @0x4d67e8: the box follows the points (or adds one of the points it was given each frame), the start of the bar pops when a
// point is added, and the numbers' effect runs
void MissionsMultiplierBox::Update(int dt) {
    BasicPage::Update(dt);
    if (m_followsSavedPoints) {
        m_multiplierLevel = (int)MissionMgr::GetInstance()->GetMultiplierLevel() + 1;
        m_shownMultiplierLevel = m_multiplierLevel;
        m_multiplierLevelPoints = (int)MissionMgr::GetInstance()->GetMultiplierLevelPoints();
        AddMultiplierPoints(0);
        SetMultiplierText(m_multiplierLevel, 1.0f);
    } else if (m_pointsToAdd > 0) {
        AddMultiplierPoints(1);
        m_pointsToAdd--;
        m_popState = 1;
    }
    if (m_popState > 0) {
        m_popTimer += dt;
        if (m_popTimer > 200) m_popState = -1;
    } else if (m_popState != 0) {
        m_popTimer += dt / -3;
    }
    if (m_popTimer <= 0) {
        m_popTimer = 0;
        m_popState = 0;
    }
    float scale = 1.0f + (float)m_popTimer * 0.001f;
    m_barStart->SetUIScale(scale);
    m_curMultiplierText->SetUIScale(scale);
    // the scaled graph keeps its middle: both are moved by twice the way its position went
    math::vec2<float> unscaled = m_barStart->GetUnscaledPos(), pos = m_barStart->GetPos();
    m_barStart->SetOffset(math::vec2<float>((unscaled.x - pos.x) + (unscaled.x - pos.x), (unscaled.y - pos.y) + (unscaled.y - pos.y)));
    unscaled = m_barStart->GetUnscaledPos();
    pos = m_barStart->GetPos();
    m_curMultiplierText->SetOffset(math::vec2<float>((unscaled.x - pos.x) + (unscaled.x - pos.x), (unscaled.y - pos.y) + (unscaled.y - pos.y)));
    UpdateMultiplierTextEffect(dt);
}

// @0x4d6a18
void MissionsMultiplierBox::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (IsVisible()) BasicPage::Render(cam, p);
}

// @0x4d6c60: the points from the level to the next one
int MissionsMultiplierBox::GetMultiplierNextLevelPoints() { return MissionMgr::GetInstance()->MissionMultiplierLevel_GetValue((unsigned)(m_multiplierLevel - 1)); }

// @0x4d6c88: the points are added (a level is gained when the next one's are reached) and the bar shows them
void MissionsMultiplierBox::AddMultiplierPoints(int points) {
    if (points) {
        m_multiplierLevelPoints += points;
        if (m_multiplierLevelPoints >= GetMultiplierNextLevelPoints()) {
            m_multiplierLevelPoints -= GetMultiplierNextLevelPoints();
            m_multiplierLevel++;
            InitMultiplierTextEffect();
        }
    }
    m_multiplierPBar->SetValue((float)m_multiplierLevelPoints / (float)GetMultiplierNextLevelPoints());
    if (m_pointsAddedCount > 1) return;  // ps::ParticleSystem::Reset() of the number's particles
    if (m_pointsAddedCount == 1) m_pointsAddedCount++;  // InterfaceText::EnableParticleSystem(true)
    else m_pointsAddedCount = 1;
}

// @0x4d6d4c: the numbers of the level and of the next one (the costumes' extra points counted), at the scale
void MissionsMultiplierBox::SetMultiplierText(int level, float scale) {
    level += CostumeMgr::GetInstance()->GetGlobalExtraDespicablePointsFactor(false);
    m_curMultiplierText->EnableAutoScroll(false);
    m_curMultiplierText->SetText(jet::String::Format("x%d", level));
    m_curMultiplierText->SetScale(math::vec2<float>(scale, scale));
    m_nextMultiplierText->EnableAutoScroll(false);
    m_nextMultiplierText->SetText(jet::String::Format("x%d", level + 1));
    m_nextMultiplierText->SetScale(math::vec2<float>(scale, scale));
}

// @0x4d6e34: the number of the new level shrinks away and grows back
void MissionsMultiplierBox::InitMultiplierTextEffect() {
    int t = m_multiplierTextEffectTimer;
    if (t <= 0) t = 450;
    else if (t >= 300) return;
    else t = 300 + (int)((float)(300 - t) / 2.0f);
    m_multiplierTextEffectTimer = t;
}

// @0x4d6e6c
void MissionsMultiplierBox::UpdateMultiplierTextEffect(int dt) {
    if (m_multiplierTextEffectTimer <= 0) return;
    m_multiplierTextEffectTimer -= dt;
    int t = m_multiplierTextEffectTimer;
    float scale;
    if (t > 0) {
        if (t > 300) scale = (float)(t - 300) * 0.0066666668f;
        else {
            scale = (float)(300 - t) * 0.0033333334f;
            m_shownMultiplierLevel = m_multiplierLevel;
        }
    } else {
        scale = 1.0f;
        m_multiplierTextEffectTimer = 0;
    }
    SetMultiplierText(m_shownMultiplierLevel, scale);
}
