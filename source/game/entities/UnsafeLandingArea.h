// source/game/entities/UnsafeLandingArea.h
#ifndef SOURCE_GAME_ENTITIES_UNSAFELANDINGAREA_H
#define SOURCE_GAME_ENTITIES_UNSAFELANDINGAREA_H

class UnsafeLandingArea : public GameEntity {  // sizeof 0x130
public:
    UnsafeLandingArea(const clara::Template*);
    virtual ~UnsafeLandingArea();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    virtual void ConstructFrom(GameEntity*);
    virtual void NotifyTransformChanged(bool);
    bool IsPointInside(const math::vec3<float>&);
private:
    void RefreshTransform();
    boost::shared_ptr<jet::video::RenderJob> GetRenderJob();
    jet::scene::Renderable *GetRenderable();
    jet::video::Material *GetMaterial();
    boost::shared_ptr<jet::video::Geometry> GetBoxGeometry();
public:
    static jet::String k_tmplName;
private:
    math::mat4<float> m_transform;  // 0x90
    math::mat4<float> m_invTransform;  // 0xD0
    float m_distanceToCheckSq;  // 0x110
    bool m_isPointInside;  // 0x114
    std::auto_ptr<jet::video::Material> m_material;  // 0x118
    boost::shared_ptr<jet::video::RenderJob> m_renderJob;  // 0x11C
    std::auto_ptr<jet::scene::RenderJobRenderable> m_renderable;  // 0x124
    boost::shared_ptr<jet::video::Geometry> m_geometryBox;  // 0x128
};

#endif
