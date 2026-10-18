// source/game/entities/Buyable.h
#ifndef SOURCE_GAME_ENTITIES_BUYABLE_H
#define SOURCE_GAME_ENTITIES_BUYABLE_H

#include "entity.h"
#include "core/pstring.h"

class Price;
class SpritePlayer;

// What the shop's bananas and tokens tabs sell ("BuyableTemplate" data entity, 0x3c bytes in the .so): a name, the price
// that says what it costs and what it gives, and the sprite of its icon. The price entities of the pack are found by the
// buyable's "price" path.
class Buyable : public clara::DataEntity {
public:
    explicit Buyable(const clara::Template*);
    // @0x5a94e8: a copy that sells for another price (dynamic pricing, which this build has none of)
    Buyable(const Buyable *original, Price *price);
    virtual ~Buyable();
    virtual void Init() {}  // @0x5a960c
    virtual void PostInit();
    void PostInit(const Buyable *original);  // @0x5a97f0
    const jet::String &GetBuyableName();
    SpritePlayer *GetSpritePlayer() const { return m_spritePlayer; }
    Price *GetPrice() const { return m_price; }
    static jet::String k_tmplName;
private:
    jet::String m_name;             // 0x20: the key of its text
    SpritePlayer *m_spritePlayer;   // 0x24: the icon
    Price *m_price;                 // 0x28
    const clara::Template *m_tmpl;  // 0x2c
    jet::String m_iconSprite;       // 0x30: the sprite of the icon
    jet::String m_iconAnim;         // 0x34: its animation, when it has one
    jet::String m_iconFrame;        // 0x38: or its frame
    static const jet::String k_name, k_price, k_icon;
};

#endif
