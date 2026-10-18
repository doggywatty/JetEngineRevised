// source/game/entities/BananaTemplate.cpp
#include "BananaTemplate.h"
#include "Minion.h"
#include "Pacesetter.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "../gamemanagers/GameSoundMgr.h"
#include "../Statistics.h"
#include "BonusSet.h"
#include "Bonus.h"
#include "param.h"
#include "project.h"

jet::String BananaTemplate::k_tmplName("BananaTemplate");
const jet::String BananaTemplate::k_comboDeco("comboDeco"), BananaTemplate::k_collectionSound("collectionSound"),
    BananaTemplate::k_collectionComboSound("CollectionComboSound"), BananaTemplate::k_collectionDistance("CollectionDistance"),
    BananaTemplate::k_magnetDistance("MagnetDistance");

// @0x5a0324
BananaTemplate::BananaTemplate(const clara::Template *t)
    : Object(t), m_state(BananaState::Alive), m_magnetizedTime(0), m_isMagnetized(false), m_comboState(false), m_collectionDistanceSq(0), m_magnetDistanceSq(0), m_pooled(false) {
    m_type = 0x15;
}

// @0x5a03b4
BananaTemplate::~BananaTemplate() {}

// @0x5a0440
void BananaTemplate::Init() {
    if (m_flags & F_LIBRARY) return;
    Object::Init();
}

// @0x5a0450
void BananaTemplate::PostInit() {
    if (m_flags & F_LIBRARY) return;
    Object::PostInit();
    GetParam(k_collectionSound, m_collectionSoundLabel);
    if (HasParam(k_collectionComboSound)) GetParam(k_collectionComboSound, m_collectionComboSoundLabel);
    if (m_collectionComboSoundLabel.empty()) m_collectionComboSoundLabel = m_collectionSoundLabel;
    if (Deco3d *combo = dynamic_cast<Deco3d*>(clara::Project::GetInstance()->FindEntityByPath(FindParamByName(k_comboDeco)->GetAsPath(0)))) combo->FillModelData(m_comboData);
    m_deco->FillModelData(m_origData);
    m_deco->RegisterForUpdate(false);
    float d = 0;
    GetParam(k_collectionDistance, d);
    m_collectionDistanceSq = d * d;
    GetParam(k_magnetDistance, d);
    m_magnetDistanceSq = d * d;
    RegisterForRender(false);
    RegisterForUpdate(true);
}

// @0x5a05f4
void BananaTemplate::PoolInstance_Prepare(bool pooled) {
    m_pooled = pooled;
    if (m_state != BananaState::Alive) {
        m_state = BananaState::Alive;
        m_deco->RegisterForUpdate(true);
        m_deco->RegisterForRender(true);
    }
    m_isMagnetized = false;
    m_comboState = false;
    m_magnetizedTime = 0;
    m_type = 0x15;
    m_deco->SetAnimationTime(0);
    RegisterForRender(false);
    RegisterForUpdate(true);
}

// @0x5a0c6c: the banana multiplier power up is on (the .so reads the status of its index, 4, in the power up manager)
bool BananaTemplate::IsBananaMultiplierActive() const { return PowerUpMgr::GetInstance()->IsPowerUpActive(EPowerUpDef::BananaMultiplier); }

// @0x5a06ec: the deco of the multiplier replaces the banana's while it is on
void BananaTemplate::CheckBananaDecoChange() {
    const bool multiplier = IsBananaMultiplierActive();
    if (multiplier) {
        if (m_comboState) return;
        m_comboState = true;
        m_deco->ApplyModelData(m_comboData);
    } else if (m_comboState) {
        m_deco->ApplyModelData(m_origData);
        m_comboState = false;
    }
}

// @0x5a0c30
void BananaTemplate::SetMagnetized(bool on) {
    if (m_isMagnetized == on) return;
    m_isMagnetized = on;
    if (on) LinkTo(0, jet::String::null, 0);
}

// @0x5a0ca8
void BananaTemplate::OnMinionRevive() {
    if (!m_isMagnetized) return;
    m_isMagnetized = false;
    m_state = BananaState::Collected;
}

// @0x5a0a60: the banana is collected: the sound (the combo one while the banana multiplier is on) at its place, the bananas counted (4 with the
// banana multiplier perk, else 2, doubled by a level of the bonus of type 11; the banana multiplier's own statistic while it is on), the run's
// "distance without bananas" reset, the bar's safe time, and the contact effects of the banana
void BananaTemplate::Die() {
    if (m_state != BananaState::Alive) return;
    m_isMagnetized = false;
    m_state = BananaState::Collected;
    const bool multiplier = IsBananaMultiplierActive();
    GameSoundMgr::GetInstance()->PlayCollideCoinSound(multiplier ? m_collectionComboSoundLabel : m_collectionSoundLabel, GetPosition());
    int bananas = PowerUpMgr::GetInstance()->HasPerk(PowerUpMgr::EPowerUpPerkDef::BananaMultiplier) ? 4 : 2;
    if (Bonus *bonus = BonusSet::GetInstance()->GetBonus(11, true))
        if (bonus->GetCurrentLevel() >= 0) bananas *= 2;
    Statistics *stats = Statistics::GetInstance();
    stats->AddBananas(bananas, true);
    stats->SetStatValue(StatisticsSaveData::k_statScope_runDistanceNoBananas, -1);
    if (IsBananaMultiplierActive()) stats->IncreaseStatistic(StatisticsSaveData::k_statScope_bananaMultiplierBananas, bananas);
    GameLevel::GetInstance()->GetMinion()->KeepDespicableBarSafe(Game::GetInstance()->GetDespBananaSafeTime());
    SpawnEffects(m_effectsOnContact, !m_effectOnContactSpawnAll);
}

// @0x5a0740: near the minion the banana updates. A banana the minion reaches is collected, unless the minion is frozen or the banana is pooled
// outside a rocket (the minion's own reaction to a collected banana is the rocket's, not ported). Within the magnet's distance, while the magnet
// is on, the banana is magnetized; a magnetized banana flies to the minion's bananas position (@0x5a0740 SetMagnetized) and is hidden once collected.
void BananaTemplate::UpdateBananaState() {
    if (m_state == BananaState::Collected) {
        m_deco->RegisterForRender(false);
        m_deco->RegisterForUpdate(false);
        RegisterForRender(false);
        m_state = BananaState::Gone;
        return;
    }
    if (m_state != BananaState::Alive) return;
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    const math::vec3<float> ref = minion->GetBananaPosition();
    const bool updating = m_deco->IsRegisteredForUpdate();
    math::vec3<float> d = ref - GetPosition();
    d.z = 0;
    const bool near = d.getLengthSq() <= 900.0f;
    if (near && !updating) m_deco->RegisterForUpdate(true);
    else if (updating && !near) m_deco->RegisterForUpdate(false);
    CheckBananaDecoChange();
    if (!minion->IsFrozenRun() && (minion->IsInRocketMode() || !m_pooled)) {
        const float distanceSq = (ref - GetPosition()).getLengthSq();
        if (distanceSq < m_collectionDistanceSq) {
            Die();
            minion->OnBananaCollect();
        } else if (!m_isMagnetized && distanceSq < m_magnetDistanceSq && PowerUpMgr::GetInstance()->IsPowerUpActive(EPowerUpDef::Magnet)) SetMagnetized(true);
    }
    // the magnetized banana flies to the minion's bananas position: the frame time (ms, times 0.001) and the factor that grows with the time it
    // has been magnetized (1 + 0.05 * ms) move it along the direction, at the pacesetter's speed times 0.15 (0 without a pacesetter)
    if (m_isMagnetized) {
        const float dt = (float)Game::GetInstance()->GetUpdateTimeUs() / 1000.0f;
        m_magnetizedTime += dt;
        Pacesetter *pacesetter = GameLevel::GetInstance()->GetMinion()->GetPacesetter();
        const float speed = pacesetter ? pacesetter->GetSpeed() : 0.0f;
        math::vec3<float> dir = ref - GetPosition();
        dir.normalize();
        const float step = dt * 0.001f * (1.0f + m_magnetizedTime * 0.05f) * (speed * 0.15f);
        SetPosition(GetPosition() + math::vec3<float>(dir.x * step, dir.y * step, dir.z * step));
    }
}

// @0x5a0670: the banana is hidden while the minion rides the rocket
void BananaTemplate::Update() {
    UpdateBananaState();
    if (m_pooled) return;
    Deco3d *deco = m_deco;
    if (GameLevel::GetInstance()->GetMinion()->IsInRocketMode()) {
        if (deco->IsVisible()) deco->SetVisible(false);
    } else if (!deco->IsVisible())
        deco->SetVisible(true);
}
