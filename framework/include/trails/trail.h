// framework/include/trails/trail.h
#ifndef FRAMEWORK_INCLUDE_TRAILS_TRAIL_H
#define FRAMEWORK_INCLUDE_TRAILS_TRAIL_H

class Trail {  // sizeof 0x1A0
public:
    Trail();
    ~Trail();
    bool GetLastSegment(math::vec3<float>&, math::vec3<float>&);
    void AddSegment(const math::vec3<float>&, const math::vec3<float>&);
    void UpdateDummyPoints(const math::vec3<float>&, const math::vec3<float>&, int);
    void SetTrailDef(const TrailDef&);
    const TrailDef &GetTrailDef();
    void Reset();
    void SetTrailDefColors(const jet::video::Color&, const jet::video::Color&);
    void Update(int);
    bool IsEmpty();
    void Clear();
    bool IsVisible();
    void SetVisible(bool);
    const boost::circular_buffer_space_optimized<Trail::Data,std::allocator<Trail::Data> > &GetData();
    float GetLength();
    const math::vec3<float> &GetWorldPosition();
    void SetWorldPosition(const math::vec3<float>&);
    void SetAutoFree(bool);
    bool GetAutoFree();
private:
    void _AddSegment(const math::vec3<float>&, const math::vec3<float>&);
public:
    static const int k_max_trailResolutionSteps;
private:
    bool m_isVisible;  // 0x0
    int m_postureTime;  // 0x4
    float m_length;  // 0x8
    int m_trailTime;  // 0xC
    TrailDef m_trailDef;  // 0x10
    boost::circular_buffer_space_optimized<Trail::Data,std::allocator<Trail::Data> > m_data;  // 0x154
    math::aabb<float> m_box;  // 0x178
    math::vec3<float> m_worldPosition;  // 0x190
    bool m_autoFree;  // 0x19C
};

#endif
