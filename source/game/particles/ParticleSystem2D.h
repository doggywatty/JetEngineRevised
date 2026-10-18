// source/game/particles/ParticleSystem2D.h
#ifndef SOURCE_GAME_PARTICLES_PARTICLESYSTEM2D_H
#define SOURCE_GAME_PARTICLES_PARTICLESYSTEM2D_H

class ParticleSystem2D {  // sizeof 0x10
public:
    ParticleSystem2D();
    ~ParticleSystem2D();
    void SetParticleSystem(boost::shared_ptr<ps::ParticleSystem>);
    void SetPixelToMetersRatio(float);
    void UpdateParticlesPosition(const math::vec3<float>&);
    void Render();
private:
    boost::shared_ptr<ps::ParticleSystem> m_particleSystem;  // 0x0
    boost::shared_ptr<jet::scene::Camera> m_camera;  // 0x8
};

#endif
