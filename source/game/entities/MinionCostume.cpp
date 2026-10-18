// source/game/entities/MinionCostume.cpp
#include "MinionCostume.h"
#include "BonusForCostumes.h"
#include "Deco3d.h"
#include "game/CGameConfig.h"
#include "game/Player.h"
#include "game/dynamicobjects/EffectMgr.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "application.h"
#include "project.h"
#include "system.h"
#include <cfloat>
#include <cmath>

jet::String MinionCostume::k_tmplName("MinionCostume");
const jet::String MinionCostume::k_buyType_buyable("buyable");
const jet::String MinionCostume::k_buyType_puzzle("puzzle");
const jet::String MinionCostume::k_costumeId("costumeId");
const jet::String MinionCostume::k_name("name");
const jet::String MinionCostume::k_bonusText("bonusText");
const jet::String MinionCostume::k_price("price");
const jet::String MinionCostume::k_orderValue("orderValue");
const jet::String MinionCostume::k_hasBonus("hasBonus");
const jet::String MinionCostume::k_bonuses("bonuses");
const jet::String MinionCostume::k_isShopElement("isShopElement");
const jet::String MinionCostume::k_shadowDeco("shopShadowDeco");
const jet::String MinionCostume::k_buyType("buyType");
const jet::String MinionCostume::k_PuzzleBGImage("PuzzleBGImage");
const jet::String MinionCostume::k_PuzzlePiecesCount("PuzzlePiecesCount");
const jet::String MinionCostume::k_PuzzlePiecesSprite("PuzzlePiecesSprite");
const jet::String MinionCostume::k_headNodeName("Bip001_Head");
const float MinionCostume::k_defaultMoveSpeed = 4.0f;
const float MinionCostume::k_baseAngle = 1.5707964f;
const float MinionCostume::k_runAngleDiff = 0.885f;
const int MinionCostume::k_maxTimeRunningInPlace = 800;
bool MinionCostume::s_isDragging = false;
float MinionCostume::s_currentBigCostumePosWorldY = 0;

// @0x57b788
MinionCostume::MinionCostume(const clara::Template *t)
    : Actor(t), m_id(0), m_price(0), m_shadowDeco(0), m_orderValue(0), m_isShopElement(false), m_enableReachTarget(false), m_hasBonus(false),
      m_targetPos(0, 0, 0), m_timeRunning(0), m_moveSpeed(k_defaultMoveSpeed) {
    SM_SetAnimatedCostume(this);
    m_type = 0x19;
}

// @0x57b874: a copy of a costume entity, under the given constructor flags
MinionCostume::MinionCostume(MinionCostume *original, unsigned ctorFlags)
    : Actor(0), m_id(0), m_price(0), m_shadowDeco(0), m_orderValue(0), m_isShopElement(false), m_enableReachTarget(false), m_hasBonus(false),
      m_targetPos(0, 0, 0), m_timeRunning(0), m_moveSpeed(k_defaultMoveSpeed) {
    m_ctorFlags = ctorFlags;
    ConstructFrom(static_cast<const GameEntity*>(original));
    m_claraEntityName = original->m_claraEntityName;
    SM_SetAnimatedCostume(this);
    m_type = 0x19;
}

// @0x57b994
MinionCostume::~MinionCostume() {
    ReleaseEffects();
    delete m_shadowDeco;
}

// @0x57bacc: a library costume only notes its name; a copy loads its body parts
void MinionCostume::Init() {
    if (IsLibraryEntity()) {
        m_claraEntityName = GetName();
        return;
    }
    Actor::Init();
    BPO_LoadBodyPartsArray(m_bodyPartDefinitions, FindParamByName(jet::String("bodyParts")));
}

// @0x57bb44: the data; a copy also gets its deco, state machine, body parts and the blob shadow
void MinionCostume::PostInit() {
    GetParam(k_costumeId, m_id);
    GetParam(k_isShopElement, m_isShopElement);
    GetParam(k_buyType, m_buyType);
    if (const clara::Param *p = FindParamByName(k_price)) {
        const clara::Path &path = p->GetAsPath(0);
        if (!path.IsEmpty()) m_price = dynamic_cast<Price*>(clara::Project::GetInstance()->FindEntityByPath(path));
    }
    GetParam(k_name, m_name);
    GetParam(k_bonusText, m_bonusText);
    GetParam(k_hasBonus, m_hasBonus);
    m_bonuses.clear();
    if (m_hasBonus)
        if (const clara::Param *p = FindParamByName(k_bonuses))
            for (unsigned i = 0; i < p->GetComponentCount(); i++) m_bonuses.push_back(static_cast<BonusForCostumes*>(p->GetAsEntity(i)));
    if (IsLibraryEntity()) return;
    SetPlaceHolderDecoIfNeeded();
    Actor::PostInit();
    BPO_Init(GetDeco() ? GetDeco()->GetModel() : 0);
    BPO_Enable(true);
    BPO_AddBodyPartsArray(m_bodyPartDefinitions);
    GetParam(k_orderValue, m_orderValue);
    // the shadow blob, unless the device draws real shadows for the menu costumes
    if (m_ctorFlags != 0 || CGameConfig::Current().useShadows <= 0)
        if (const clara::Param *p = FindParamByName(k_shadowDeco))
            if (Deco3d *ref = dynamic_cast<Deco3d*>(clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(0)))) {
                m_shadowDeco = new Deco3d(ref, m_ctorFlags);
                m_shadowDeco->LinkTo(this, jet::String::null, 0);
            }
    m_targetPos = GetPosition();
    if (m_ctorFlags & EGEConstructorFlagsDef::Interface) SetEffectLayerId(EffectMgr::GetNextLayerId());
}

// @0x57be78
void MinionCostume::Update() {
    Actor::Update();
    const int dt = (int)(jet::System::GetApplication()->GetUpdateTimeUs() / 1000);
    BPO_Update(dt);
    UpdateReachTarget((unsigned)dt);
    if (m_shadowDeco) m_shadowDeco->Update();
}

// @0x57bee4: walks along y to the target (turned towards it, in the run state 11); once there turns to face the
// camera and goes back to the idle label 1. While a costume is dragged it keeps running in place a while (600 ms
// before turning, 800 before stopping).
void MinionCostume::UpdateReachTarget(unsigned dt) {
    if (!m_enableReachTarget) return;
    math::vec3<float> pos = GetPosition();
    const ActorState *state = SM_GetState();
    const bool running = state && state->m_label == 3;
    if (!math::equals(m_targetPos, pos, 1.1920929e-07f)) {
        const float dy = m_targetPos.y - pos.y;
        float step = jet::System::GetApplication()->GetUpdateTimeSeconds() * m_moveSpeed;
        if (std::fabs(dy) < step) {
            if (s_isDragging) goto reached;
            SetPosition(m_targetPos);
            return;
        }
        if (std::fabs(dy) > step) {
            if (!running) {
                m_timeRunning = 0;
                SM_SetStateLabel(11);
            }
            const math::quat<float> q(math::quat<float>::axisZ, dy > 0 ? k_baseAngle + k_runAngleDiff : k_baseAngle - k_runAngleDiff);
            GetDeco()->SetRotationToReach(q, 300);
        }
        if (dy <= 0) step = -step;
        pos.y += step;
        SetPosition(pos);
        return;
    }
reached:
    if (!running) return;
    if (s_isDragging) {
        m_timeRunning += (int)dt;
        if (m_timeRunning <= 600) return;
        GetDeco()->SetRotationToReach(math::quat<float>(math::quat<float>::axisZ, k_baseAngle), 1000);
        if (m_timeRunning <= k_maxTimeRunningInPlace) return;
    } else {
        GetDeco()->SetRotationToReach(math::quat<float>(math::quat<float>::axisZ, k_baseAngle), 1000);
    }
    SM_SetStateLabel(1);
}

// @0x57c0f4: a touched body part sends its touch event to the state machine
void MinionCostume::BPO_OnBodyPartTouched(BodyPartSection *s) { SM_OnEngineEvent(s->def->touchEvent + 0xf); }

// @0x57c144: the effects on the model go (when there is an effect manager), the state tracks are reset
void MinionCostume::ReleaseEffects() {
    if (EffectMgr *mgr = EffectMgr::GetInstance())
        if (m_deco) mgr->ReleaseAutoRemoveEffectsAttachedToModel(m_deco->GetModel());
    SM_ResetStateTracks();
}

// @0x57c194
const jet::String &MinionCostume::GetGameName() const { return babel::Babel::GetInstance()->GetStringMgr()->Get(m_name); }

// @0x57c1c8
const jet::String &MinionCostume::GetBonusText() { return babel::Babel::GetInstance()->GetStringMgr()->Get(m_bonusText); }

// @0x4bffec
bool MinionCostume::HasPromo() { return m_price->HasPricePromotion() || m_price->GetExtraCategoryCount() > 0; }

// @0x57c648
void MinionCostume::SetEffectLayerId(int id) {
    Object::m_effectLayerId = id;
    StateMachine::m_effectLayerId = id;
}

// @0x57c528: an active price not bought yet (the magician costume, 25, only once the magic costume offer is on)
bool MinionCostume::ShouldBeShownInShop() {
    const bool active = m_price ? m_price->IsActive() : false;
    const bool bought = CostumeMgr::GetInstance()->WasBought(*this);
    if (m_id == 0x19 && Player::GetInstance()->IsMigicCostume() == -1) return bought;
    if (bought) return bought;
    return m_isShopElement && active;
}

// @0x4bff7c: within a frame's walk (at 30 fps) of the big costume's place
bool MinionCostume::IsNearTheCurrentPos() {
    return k_defaultMoveSpeed * (1.0f / 30.0f) >= std::fabs(GetPosition().y - s_currentBigCostumePosWorldY);
}

// @0x4826d4
bool MinionCostume::HasReachedTargetPos() {
    const math::vec3<float> &p = GetPosition();
    return math::equals(m_targetPos.x, p.x, 1.1920929e-07f) && math::equals(m_targetPos.y, p.y, 1.1920929e-07f) && math::equals(m_targetPos.z, p.z, 1.1920929e-07f);
}

// @0x57c5c0
bool MinionCostume::IsCurrentlyEquipped() { return m_claraEntityName.Equals(CostumeMgr::GetInstance()->GetEquippedCostumeName()); }

// @0x57c654: the price is in the store's "TIME" extra category
bool MinionCostume::IsTimeLimited() {
    const jet::String timeLimited("TIME");
    for (int i = 0; i < m_price->GetExtraCategoryCount(); i++)
        if (m_price->GetExtraCategory(i).Equals(timeLimited)) return true;
    return false;
}

// @0x57c5f8: a costume whose model is not in the data shows the placeholder costume's
void MinionCostume::SetPlaceHolderDecoIfNeeded() {
    if (CanLoadModel()) return;
    if (MinionCostume *ph = CostumeMgr::GetInstance()->GetPlaceHolderCostumeRef()) m_defaultDeco = ph->GetDecoReference(0, false);
}

// @0x57c1fc: each skill 8 bonus takes its amount (percent) off
float MinionCostume::GetFriendChallengeEasierFactor() const {
    float f = 1.0f;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 8) f -= b->GetAmountAsFloat() * 0.01f;
    return f;
}

// @0x57c24c
int MinionCostume::GetExtraBananasOnFriendsChallenge() const {
    int n = 0;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 5) n += b->GetAmountAsInt();
    return n;
}

// @0x57c294: each skill 4 bonus multiplies by its amount (percent)
float MinionCostume::GetExtraBananasFactor() const {
    float f = 1.0f;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 4) f *= b->GetAmountAsFloat() * 0.01f;
    return f;
}

// @0x57c2e8: seconds (as ms) more for the power-ups of the type
int MinionCostume::GetExtraTimeForPowerUp(GameItemType t) const {
    int ms = 0;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 3 && IsItemTypeEquivalent(t, b->GetGameItemType())) ms += 1000 * b->GetAmountAsInt();
    return ms;
}

// @0x57c34c
int MinionCostume::RevivesForFree() const {
    int n = 0;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 1) n += b->GetAmountAsInt();
    return n;
}

// @0x57c394
bool MinionCostume::FreezeRaySmashesObstacleApart() const {
    bool r = false;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 9) r = true;
    return r;
}

// @0x57c3d4
int MinionCostume::GetExtraDespicablePointsFactorWhenRunning() const {
    int n = 0;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 6) n += b->GetAmountAsInt();
    return n;
}

// @0x57c420: a costume bought with tokens (a price amount of at most 100) adds that much to the global factor
int MinionCostume::GetExtraDespicablePointsFactorForGlobal() const {
    if (!m_price || !(std::fabs(m_price->GetPriceAsDouble()) > DBL_EPSILON)) return 0;
    if (m_price->GetAmount() > 100) return 0;
    return m_price->GetAmount();
}

// @0x57c480
int MinionCostume::GetDespicableActionFactor() const {
    int f = 1;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 7) f *= b->GetAmountAsInt();
    return f;
}

// @0x57c4c8
int MinionCostume::GetExtraWeightFactorForPowerUp(GameItemType t) const {
    int n = 0;
    for (BonusForCostumes *b : m_bonuses)
        if (b->GetSkillType() == 2 && IsItemTypeEquivalent(t, b->GetGameItemType())) n += b->GetAmountAsInt();
    return n;
}

// @0x57c710
int MinionCostume::GetPuzzlePiecesCount() const {
    int n = 0;
    GetParam(k_PuzzlePiecesCount, n);
    return n;
}

// @0x57c740
const jet::String &MinionCostume::GetPuzzleSpriteName() const {
    const clara::Param *p = FindParamByName(k_PuzzlePiecesSprite);
    return p ? p->GetAsSprite(0).file : jet::String::null;
}

// @0x57c784
const jet::String &MinionCostume::GetPuzzleBGImageSpriteName() const {
    const clara::Param *p = FindParamByName(k_PuzzleBGImage);
    return p ? p->GetAsSprite(0).file : jet::String::null;
}

// @0x57c7c8
int MinionCostume::GetPuzzleBGImageSpriteFrameId() const {
    const clara::Param *p = FindParamByName(k_PuzzleBGImage);
    if (!p || p->GetAsSprite(0).anim.empty()) return -1;
    return atoi(p->GetAsSprite(0).anim.c_str());
}

// @0x57c81c
int MinionCostume::GetPuzzleBGImageSpriteAnimId() const {
    const clara::Param *p = FindParamByName(k_PuzzleBGImage);
    if (!p || p->GetAsSprite(0).frame.empty()) return -1;
    return atoi(p->GetAsSprite(0).frame.c_str());
}
