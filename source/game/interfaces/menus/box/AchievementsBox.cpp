// source/game/interfaces/menus/box/AchievementsBox.cpp
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "AchievementsBox.h"
#include "game/Player.h"
#include "game/entities/Achievement.h"
#include "game/entities/Mission.h"
#include "game/gamemanagers/AchievementsMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"

// @0x4cd884: the achievement is brought up to date; its background says whether it is done, the claim button whether its
// prize is still to take, and the bar how far it is
AchievementsBox::AchievementsBox(Achievement *achievement) : BasicPage(jet::String("AchievementsTab")), m_achievement(achievement), m_claimButton(0) {
    InterfaceObject::SetSize(GetUIObject(jet::String("TabSize"))->GetSize());
    m_achievement->ForceConditionUpdate();
    GetUIObject(jet::String("TabBg_locked"))->SetVisible(!m_achievement->IsCompleted());
    GetUIObject(jet::String("TabBg_achieved"))->SetVisible(m_achievement->IsCompleted());
    const bool claimed = AchievementsMgr::GetInstance()->WasPrizeClaimedFor(m_achievement->GetName());
    m_claimButton = GetUIButton(jet::String("ClaimPrize"));
    m_claimButton->SetVisible(!claimed);
    InterfacePBar *bar = GetUIPBar(jet::String("InterfacePBar"));
    bar->SetValue(0.0f);
    bar->Update(1);
    bar->EnableInertia(true, InterfacePBar::k_defaultInertiaMultiPerMillisecond);
    float percentage = m_achievement->GetCompletedPercentage();
    // what only counts in the current run starts again from nothing
    if (m_achievement->GetConditionsScope().Equals(Mission::k_missionScopeCurrentRun) && !m_achievement->IsCompleted()) percentage = 0.0f;
    bar->SetValue(percentage);
    Refresh();
}

AchievementsBox::~AchievementsBox() {}

// @0x4cdd84: the claim button gives the prize
void AchievementsBox::Update(int dt) {
    BasicPage::Update(dt);
    AchievementsMgr *mgr = AchievementsMgr::GetInstance();
    mgr->MarkIGNotifyShownFor(m_achievement->GetName());
    MenuMgr *mm = MenuMgr::GetInstance();
    const int count = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < count; i++) {
        if (m_claimButton != mm->ACTION_GetReleasedButton(i)) continue;
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
        m_claimButton->SetVisible(false);
        mgr->SetPrizeClaimedFor(m_achievement->GetName());
        Player::GetInstance()->AddAmountFrom(*m_achievement->GetPrize());
        // GameTrackingMgr::SendAchievementsUnlockedThroughGamePlayEvent(id, 0x1c61c, tokens given), GameCrmMgr::AchievementUnlockedPointCut(id)
        return;
    }
}

// @0x4cdec4
void AchievementsBox::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (!IsVisible()) return;
    BasicPage::Render(cam, p);
}

// @0x4cdefc: the name with what is done of it (nothing once it is), the text, and the prize of bananas or of tokens
void AchievementsBox::Refresh() {
    InterfaceText *name = GetUIText(jet::String("AchievementName"));
    int done = 0, total = 0;
    m_achievement->GetCompletedInt(done, total);
    const jet::String title = m_achievement->GetAchievementName();
    const jet::String progress = m_achievement->IsCompleted() ? jet::String("") : jet::String::Format(" %d/%d", done, total);
    name->SetText(title + progress);
    GetUIText(jet::String("Tx_Description"))->SetText(m_achievement->GetAchievementDescription());
    const Price *prize = m_achievement->GetPrize();
    const jet::String amount = babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(prize->GetAmount());
    const bool bananas = prize->GetPriceType() == Price::Bananas, tokens = prize->GetPriceType() == Price::Tokens;
    GetUIObject(jet::String("Claim_Icon_bananas"))->SetVisible(bananas);
    GetUIObject(jet::String("Claim_Icon_tokens"))->SetVisible(tokens);
    GetUIText(jet::String("Claim_Bananas_value"))->SetText(amount);
    GetUIObject(jet::String("Prize_Icon_bananas"))->SetVisible(bananas);
    GetUIObject(jet::String("Prize_Icon_tokens"))->SetVisible(tokens);
    GetUIText(jet::String("Prize_value"))->SetText(jet::String("x") + amount);
}
