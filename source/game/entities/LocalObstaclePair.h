// source/game/entities/LocalObstaclePair.h
#ifndef SOURCE_GAME_ENTITIES_LOCALOBSTACLEPAIR_H
#define SOURCE_GAME_ENTITIES_LOCALOBSTACLEPAIR_H

class LocalObstaclePair : public GameEntity {  // sizeof 0x98
public:
    LocalObstaclePair(const clara::Template*);
    virtual ~LocalObstaclePair();
    virtual void Init();
    int GetLocalizationId();
    ObstacleTemplate *GetObstacleTemplate();
    static jet::String k_tmplName;
private:
    int m_localizationId;  // 0x90
    ObstacleTemplate *m_obstacleTemplate;  // 0x94
    static const jet::String k_localizationId;
    static const jet::String k_localObstacleRef;
};

#endif
