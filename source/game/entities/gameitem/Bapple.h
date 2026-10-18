// source/game/entities/gameitem/Bapple.h
#ifndef SOURCE_GAME_ENTITIES_GAMEITEM_BAPPLE_H
#define SOURCE_GAME_ENTITIES_GAMEITEM_BAPPLE_H

#include "CollectableItem.h"

class Deco3d;

// A bapple on the way (the data's "Bapple", 0xDC bytes in the .so): the objective of the bapple game. Collected it tells the BappleMgr
// (OnBappleCollected, which is not ported yet).
class Bapple : public CollectableItem {
public:
    explicit Bapple(const clara::Template*);
    virtual ~Bapple();
    virtual void Init();
    virtual void PostInit();
    virtual bool CanBeCollectedInVehicle() const { return true; }
    const Deco3d *GetShopDecoRef() const { return m_shopDecoRef; }
    static jet::String k_tmplName;
    static const jet::String k_shopDecoParam, k_idleEffectParam;
protected:
    virtual void Collect(Minion*);
private:
    const Deco3d *m_shopDecoRef;      // 0xCC
    clara::DataEntity *m_idleEffect;  // 0xD4 (an EffectDef)
};

#endif
