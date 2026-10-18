// source/game/entities/Billboard.h
#ifndef SOURCE_GAME_ENTITIES_BILLBOARD_H
#define SOURCE_GAME_ENTITIES_BILLBOARD_H

class Billboard : public Deco3d {  // sizeof 0x1FC
public:
    virtual const jet::String &GetBaseTemplateName();
    Billboard(const clara::Template*);
    virtual ~Billboard();
    virtual void Init();
    virtual void PostInit();
    virtual void Render();
    static jet::String k_tmplName;
protected:
    bool m_axialBillboarding;  // 0x1DC
    math::vec3<float> m_alignedAxis;  // 0x1E0
    math::quat<float> m_alignedCorrectionRot;  // 0x1EC
    static const jet::String k_axialBillboarding;
    static const jet::String k_alignedAxis;
};

#endif
