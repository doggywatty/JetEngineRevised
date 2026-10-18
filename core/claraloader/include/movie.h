// core/claraloader/include/movie.h
#ifndef CORE_CLARALOADER_INCLUDE_MOVIE_H
#define CORE_CLARALOADER_INCLUDE_MOVIE_H

#include "smath.h"
#include "clara.h"

namespace clara {

struct PositionKF { int time; unsigned interp; math::vec3<float> value; };
struct RotationKF { int time; unsigned interp; math::quat<float> value; };
struct ScaleKF { int time; unsigned interp; math::vec3<float> value; };
struct AnimationKF { int time; jet::String name; unsigned start, loops; bool playing; };
struct ScriptKF { int time; jet::String function, args; };

// Cutscene data: tracks of keyframes, the entity tracks bound to entity paths.
class Movie {
public:
    enum TrackType { TRACK_ENTITY, TRACK_X, TRACK_SOUND, TRACK_MUSIC, TRACK_PARTICLE, TRACK_B };

    struct TrackData {
        TrackType type = TRACK_ENTITY;
        Path path;
        Entity* entity = nullptr;
        std::vector<PositionKF> position;
        std::vector<RotationKF> rotation;
        std::vector<ScaleKF> scale;
        std::vector<AnimationKF> animation;
        std::vector<ScriptKF> script;
    };

    Movie() : m_fps(30), m_msPerFrame(33), m_start(0), m_end(0), m_time(0), m_loop(false), m_playing(false) {}
    Movie(const Movie&) = delete;
    Movie& operator=(const Movie&) = delete;
    Movie(Movie&&) = default;

    void Load(jet::stream::IStream& s);
    void PostInit(Project& p);
    void Unload() { m_tracks.clear(); }
    void Update(int dtMs);
    void Animate(int timeMs);

    const jet::String& GetName() const { return m_name; }
    unsigned GetFps() const { return m_fps; }
    int GetStart() const { return m_start; }
    int GetEnd() const { return m_end; }
    int GetTime() const { return m_time; }
    bool IsLooping() const { return m_loop; }
    bool IsPlaying() const { return m_playing; }
    void SetPlaying(bool on) { m_playing = on; }
    void SetLooping(bool on) { m_loop = on; }
    void SetTime(int t) { m_time = t < m_start ? m_start : t > m_end ? m_end : t; m_accumMs = 0; }
    unsigned GetTrackCount() const { return unsigned(m_tracks.size()); }
    const TrackData& GetTrack(unsigned i) const { return m_tracks[i]; }

private:
    void LoadTrack(jet::stream::IStream& s, unsigned i);

    jet::String m_name;
    unsigned m_fps;
    unsigned m_msPerFrame;
    int m_start, m_end, m_time;
    int m_accumMs = 0;  // the time of the frames not made yet
    bool m_loop, m_playing;
    std::vector<TrackData> m_tracks;
};

}

#endif
