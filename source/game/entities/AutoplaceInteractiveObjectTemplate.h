// source/game/entities/AutoplaceInteractiveObjectTemplate.h
#ifndef SOURCE_GAME_ENTITIES_AUTOPLACEINTERACTIVEOBJECTTEMPLATE_H
#define SOURCE_GAME_ENTITIES_AUTOPLACEINTERACTIVEOBJECTTEMPLATE_H

class AutoplaceInteractiveObjectTemplate : public InteractiveObjectTemplate {  // sizeof 0x300
public:
    AutoplaceInteractiveObjectTemplate(const clara::Template*);
    virtual ~AutoplaceInteractiveObjectTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render();
    void SetSkin(const boost::shared_ptr<jet::video::Texture>&, const jet::String&);
    void SetGameDistance(unsigned);
    void SetRealDistance(float);
    bool IsDesiredDistanceSetted();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    static jet::String k_tmplName;
protected:
    std::map<jet::String,boost::shared_ptr<jet::video::Texture>,std::less<jet::String>,std::allocator<std::pair<jet::String const ,boost::shared_ptr<jet::video::Texture> > > > m_texturePtrBackup;  // 0x2D4
    jet::String m_skinMaterialName;  // 0x2F0
    float m_desiredRealDistance;  // 0x2F4
    float m_placedRealDistance;  // 0x2F8
    bool m_started;  // 0x2FC
private:
    static const int k_distanceToDelete;
    static const jet::String k_defaultSkinningMaterialName;
    static const jet::String k_autoplaceGameDistanceParam;
};

#endif
