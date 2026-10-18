// source/game/entities/gameitem/Token.h
#ifndef SOURCE_GAME_ENTITIES_GAMEITEM_TOKEN_H
#define SOURCE_GAME_ENTITIES_GAMEITEM_TOKEN_H

#include "CollectableItem.h"

class Deco3d;

// A token on the way (the data's "Token", 0xE8 bytes in the .so): collected it counts as one more of the player's. The one a day (the
// DailyTokenMgr's) tells the manager first, which joins when that manager is ported.
class Token : public CollectableItem {
public:
    explicit Token(const clara::Template*);
    virtual ~Token();
    virtual void Init();
    virtual void PostInit();
    const Deco3d *GetShopDecoRef() const { return m_shopDecoRef; }
    static jet::String k_tmplName;
    static const jet::String k_shopDeco;
protected:
    virtual void Collect(Minion*);
private:
    bool m_isDailyTokenGenerated;  // 0xCC
    const Deco3d *m_shopDecoRef;   // 0xD0
};

#endif
