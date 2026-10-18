// framework/include/trails/trailmgr.h
#ifndef FRAMEWORK_INCLUDE_TRAILS_TRAILMGR_H
#define FRAMEWORK_INCLUDE_TRAILS_TRAILMGR_H

class TrailMgr : public Singleton<TrailMgr> {  // sizeof 0x1A1D8
public:
    TrailMgr();
    virtual ~TrailMgr();
    void Update(int);
    void Render(jet::scene::Camera*);
    void Render2D();
    Trail *GetTrail();
    void FreeTrail(Trail*, bool);
    void FreeAllTrails();
    const TrailCache *GetTrailCache(int);
    TrailCache *CreateTrailCache(int);
    void UpdateDebugContext();
private:
    void _Render(bool, jet::scene::Camera*);
    Trail m_trailPool[256];  // 0x4
    bool m_isTrailUsed[256];  // 0x1A004
    boost::unordered::unordered_map<int,TrailCache,boost::hash<int>,std::equal_to<int>,std::allocator<std::pair<int const ,TrailCache> > > m_trailCacheMap;  // 0x1A104
    ustl::vector<jet::video::Painter::Vertex> m_vertices;  // 0x1A120
    dbg::DebugContext m_dbgCtx;  // 0x1A130
    unsigned m_dips;  // 0x1A1D0
    jet::video::Painter *m_painter;  // 0x1A1D4
};

#endif
