// source/game/entities/Tilemap.h
#ifndef SOURCE_GAME_ENTITIES_TILEMAP_H
#define SOURCE_GAME_ENTITIES_TILEMAP_H

class Tilemap : public GameEntity {  // sizeof 0x9C
public:
    Tilemap(const clara::Template*);
    virtual ~Tilemap();
    virtual void Init();
    virtual void PostInit();
    short GetCollisionTilemapH();
    short GetCollisionTilemapW();
    unsigned char GetCollisionData(unsigned, unsigned);
private:
    void LoadTilemapData();
public:
    static jet::String k_tmplName;
private:
    jet::String m_tilemapName;  // 0x90
    short m_collisionTilemapW;  // 0x94
    short m_collisionTilemapH;  // 0x96
    signed char* m_collisionData;  // 0x98
    static const jet::String k_tilemapNameParam;
};

#endif
