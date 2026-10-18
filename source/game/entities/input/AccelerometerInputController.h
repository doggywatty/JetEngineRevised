// source/game/entities/input/AccelerometerInputController.h
#ifndef SOURCE_GAME_ENTITIES_INPUT_ACCELEROMETERINPUTCONTROLLER_H
#define SOURCE_GAME_ENTITIES_INPUT_ACCELEROMETERINPUTCONTROLLER_H

class AccelerometerInputController : public PlayerInputController {  // sizeof 0xA4
public:
    AccelerometerInputController(const clara::Template*);
    virtual ~AccelerometerInputController();
    virtual void Init();
    virtual void Update(Minion*);
    virtual void OnEnabled();
    virtual void OnDisabled();
    void UpdateSensitivity();
    virtual void NotifyParamChanged(const jet::String&);
private:
    float GetRealTiltAngle();
    float GetSteering(float);
public:
    static jet::String k_tmplName;
private:
    float m_tiltInputLimitRad;  // 0x90
    float m_tiltInputLimitRadMin;  // 0x94
    float m_tiltInputLimitRadMax;  // 0x98
    float m_tiltSensitivity;  // 0x9C
    float m_accumulativeSteering;  // 0xA0
};

#endif
