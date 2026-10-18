// source/game/dynamicobjects/TrailDefEntity.h
#ifndef SOURCE_GAME_DYNAMICOBJECTS_TRAILDEFENTITY_H
#define SOURCE_GAME_DYNAMICOBJECTS_TRAILDEFENTITY_H

class TrailDefEntity : public GameEntity {  // sizeof 0x1D4
public:
    TrailDefEntity(const clara::Template*);
    virtual ~TrailDefEntity();
    virtual void Init();
    virtual void PostInit();
    TrailDef &GetTrailDef();
private:
    TrailDef m_trailDef;  // 0x90
public:
    static jet::String k_tmplName;
};

#endif
