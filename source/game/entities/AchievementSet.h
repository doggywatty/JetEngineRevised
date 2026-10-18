// source/game/entities/AchievementSet.h
#ifndef SOURCE_GAME_ENTITIES_ACHIEVEMENTSET_H
#define SOURCE_GAME_ENTITIES_ACHIEVEMENTSET_H

#include <vector>
#include "GameEntity.h"

class Achievement;

// The list of the achievements of the game (the "AchievementSet" entity of the level; there is one)
class AchievementSet : public GameEntity {
public:
    explicit AchievementSet(const clara::Template*);          // @0x59e6e0
    virtual ~AchievementSet() { if (s_instance == this) s_instance = 0; }
    virtual void PostInit();                                  // @0x59e7c8
    void OnAchievementCompleted(Achievement*);                // @0x59e924
    static AchievementSet *GetInstance() { return s_instance; }  // @0x59e988
    static bool HasInstance() { return s_instance != 0; }
    std::vector<Achievement*> &GetAchievements() { return m_achievements; }  // @0x59e980
    static const jet::String k_tmplName;
private:
    std::vector<Achievement*> m_achievements;  // 0x90
    static AchievementSet *s_instance;
};

#endif
