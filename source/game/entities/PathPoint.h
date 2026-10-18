// source/game/entities/PathPoint.h
#ifndef SOURCE_GAME_ENTITIES_PATHPOINT_H
#define SOURCE_GAME_ENTITIES_PATHPOINT_H

class PathPoint : public GameEntity {  // sizeof 0x90
public:
    PathPoint(const clara::Template*);
    virtual ~PathPoint();
    static jet::String k_tmplName;
};

#endif
