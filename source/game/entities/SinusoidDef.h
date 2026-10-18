// source/game/entities/SinusoidDef.h
#ifndef SOURCE_GAME_ENTITIES_SINUSOIDDEF_H
#define SOURCE_GAME_ENTITIES_SINUSOIDDEF_H

class SinusoidDef : public GameEntity {  // sizeof 0x98
public:
    SinusoidDef(const clara::Template*);
    virtual ~SinusoidDef();
    virtual void Init();
    float GetValue(float);
    static jet::String k_tmplName;
private:
    float m_amplitud;  // 0x90
    float m_frequency;  // 0x94
    static const jet::String k_amplitudParam;
    static const jet::String k_frequencyParam;
};

#endif
