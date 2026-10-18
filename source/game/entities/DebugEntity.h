// source/game/entities/DebugEntity.h
#ifndef SOURCE_GAME_ENTITIES_DEBUGENTITY_H
#define SOURCE_GAME_ENTITIES_DEBUGENTITY_H

class DebugEntity : public GameEntity {  // sizeof 0x98
public:
    DebugEntity(const clara::Template*);
    virtual ~DebugEntity();
    virtual void PostInit();
    virtual void Render();
    void SetMessage(const jet::String&);
    void SetColor(unsigned);
    static jet::String k_tmplName;
private:
    jet::String m_message;  // 0x90
    unsigned m_color;  // 0x94
};

#endif
