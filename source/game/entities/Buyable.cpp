// source/game/entities/Buyable.cpp
#include "Buyable.h"
#include "game/gamemanagers/BuyablesMgr.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "project.h"
#include "sprites/sprite.h"
#include "sprites/spritemgr.h"
#include "sprites/spriteplayer.h"
#include <cstdlib>

jet::String Buyable::k_tmplName("BuyableTemplate");  // @0xc43966
const jet::String Buyable::k_name("name");           // @0xc371e3
const jet::String Buyable::k_price("price");         // @0xc3dd5a
const jet::String Buyable::k_icon("icon");           // @0xc43976

// @0x5a9490
Buyable::Buyable(const clara::Template *t) : clara::DataEntity(t), m_spritePlayer(0), m_price(0), m_tmpl(t) {}

// @0x5a94e8
Buyable::Buyable(const Buyable *original, Price *price) : clara::DataEntity(original->m_tmpl), m_spritePlayer(0), m_price(price), m_tmpl(original->m_tmpl) {}

// @0x5a954c
Buyable::~Buyable() { delete m_spritePlayer; }

// @0x5a9610: the name, the price entity (with the text of the buyable as its description), the icon, and into the
// buyables' list
void Buyable::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(k_name, m_name);
    if (const clara::Param *p = FindParamByName(k_price)) {
        const clara::Path &path = p->GetAsPath(0);
        if (!path.IsEmpty()) {
            m_price = dynamic_cast<Price*>(clara::Project::GetInstance()->FindEntityByPath(path));
            if (m_price) m_price->SetItemDescription(babel::Babel::GetInstance()->GetStringMgr()->Get(m_name));
        }
    }
    const clara::Param::SpriteValue &icon = FindParamByName(k_icon)->GetAsSprite(0);
    m_spritePlayer = new SpritePlayer(icon.file, 0, 0, 0);
    m_iconSprite = icon.file;
    // a frame when there is one, else the animation
    if (icon.anim.empty()) {
        m_spritePlayer->SetAnim(atoi(icon.frame.c_str()), true);
        m_iconAnim = icon.frame;
    } else {
        m_spritePlayer->SetFrame(atoi(icon.anim.c_str()));
        m_iconFrame = icon.anim;
    }
    BuyablesMgr::GetInstance()->AddBuyable(this);
}

// @0x5a97f0
void Buyable::PostInit(const Buyable *original) {
    clara::DataEntity::PostInit();
    m_spritePlayer = new SpritePlayer(original->m_iconSprite, 0, 0, 0);
    m_iconSprite = original->m_iconSprite;
    if (original->m_iconFrame.empty()) {
        m_spritePlayer->SetAnim(atoi(original->m_iconAnim.c_str()), true);
        m_iconAnim = original->m_iconAnim;
    } else {
        m_spritePlayer->SetFrame(atoi(original->m_iconFrame.c_str()));
        m_iconFrame = original->m_iconFrame;
    }
    BuyablesMgr::GetInstance()->AddBuyable(this);
}

// @0x4dab58
const jet::String &Buyable::GetBuyableName() { return babel::Babel::GetInstance()->GetStringMgr()->Get(m_name); }
