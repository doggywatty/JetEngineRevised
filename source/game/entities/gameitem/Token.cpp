// source/game/entities/gameitem/Token.cpp
#include "Token.h"
#include "../Deco3d.h"
#include "../../Player.h"
#include "../../Statistics.h"
#include "param.h"
#include "project.h"

jet::String Token::k_tmplName("Token");
const jet::String Token::k_shopDeco("shopDeco");

// @0x5680d4
Token::Token(const clara::Template *t) : CollectableItem(t), m_isDailyTokenGenerated(false), m_shopDecoRef(0) { m_type = 4; }

// @0x568120
Token::~Token() {}

// @0x56823c
void Token::Init() { CollectableItem::Init(); }

// @0x56824c
void Token::PostInit() {
    clara::Path path;
    if (const clara::Param *p = FindParamByName(k_shopDeco)) path = p->GetAsPath(0);
    m_shopDecoRef = dynamic_cast<Deco3d*>(clara::Project::GetInstance()->FindEntityByPath(path));
    if (m_flags & F_LIBRARY) return;
    RegisterForUpdate(true);
    RegisterForRender(false);
    CollectableItem::PostInit();
}

// @0x568188: one more token (DailyTokenMgr::OnTokenCollected, for the one of the day, joins with that manager)
void Token::Collect(Minion*) {
    m_state = CollectableStateDef::Collected;
    Statistics::GetInstance()->IncreaseLocationStat(-1, StatisticsSaveData::k_statScope_tokens, 1);
    Player::GetInstance()->AddTokens(1);
}
