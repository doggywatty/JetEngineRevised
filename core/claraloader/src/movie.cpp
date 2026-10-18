// core/claraloader/src/movie.cpp
#include "movie.h"
#include <cstdio>
#include "entity.h"
#include "mem/memorymanager.h"
#include "project.h"

namespace clara {

// name, u32 fps, u32 start, u32 end, bool loop, u32 tracks, tracks.
void Movie::Load(jet::stream::IStream& s)
{
    jet::mem::Tag tag("clara");
    u32 start = 0, end = 0, n = 0;
    s >> m_name >> m_fps >> start >> end >> m_loop;
    m_msPerFrame = m_fps ? 1000 / m_fps : 0;
    m_start = int(start);
    m_end = int(end);
    m_time = m_start < m_end ? m_start : m_end;
    m_playing = false;
    s >> n;
    m_tracks.assign(n, TrackData());
    for (unsigned i = 0; i < n; ++i) {
        char t = 0;
        s.Read(&t, 1);
        switch (t) {
        case 'e': m_tracks[i].type = TRACK_ENTITY; break;
        case 'x': m_tracks[i].type = TRACK_X; break;
        case 's': m_tracks[i].type = TRACK_SOUND; break;
        case 'm': m_tracks[i].type = TRACK_MUSIC; break;
        case 'p': m_tracks[i].type = TRACK_PARTICLE; break;
        case 'b': m_tracks[i].type = TRACK_B; break;
        default: printf("Movie '%s': unknown track type '%c'\n", m_name.c_str(), t);
        }
        LoadTrack(s, i);
    }
}

// Entity tracks start with the entity path. Then frames: s32 time (negative ends), u16 mask,
// and per set bit: 1 position, 2 rotation, 4 scale (each u8 interpolation + value),
// 8 animation (name, u32 start, u32 loops, bool), 16 script (function, args).
void Movie::LoadTrack(jet::stream::IStream& s, unsigned i)
{
    TrackData& t = m_tracks[i];
    if (t.type == TRACK_ENTITY)
        s >> t.path;
    int time = 0;
    s >> time;
    while (time >= 0) {
        u16 mask = 0;
        s >> mask;
        if (mask & 1) {
            PositionKF k{time, 0, {}};
            u8 in = 0;
            s >> in >> k.value;
            k.interp = in;
            t.position.push_back(k);
        }
        if (mask & 2) {
            RotationKF k{time, 0, {}};
            u8 in = 0;
            s >> in >> k.value;
            k.interp = in;
            t.rotation.push_back(k);
        }
        if (mask & 4) {
            ScaleKF k{time, 0, {}};
            u8 in = 0;
            s >> in >> k.value;
            k.interp = in;
            t.scale.push_back(k);
        }
        if (mask & 8) {
            AnimationKF k{time, jet::String(), 0, 0, false};
            s >> k.name >> k.start >> k.loops >> k.playing;
            t.animation.push_back(k);
        }
        if (mask & 16) {
            ScriptKF k{time, jet::String(), jet::String()};
            s >> k.function >> k.args;
            t.script.push_back(k);
        }
        s >> time;
    }
}

void Movie::PostInit(Project& p)
{
    for (TrackData& t : m_tracks)
        if (t.type == TRACK_ENTITY) {
            DataEntity* e = p.FindEntityByPath(t.path);
            t.entity = e && e->RttiIsOfType(Entity::RttiGetClassId()) ? static_cast<Entity*>(e) : nullptr;
        }
}

template <class K, class V, class L>
static bool Sample(const std::vector<K>& keys, int time, V& out, L lerp)
{
    if (keys.empty())
        return false;
    if (time <= keys.front().time) {
        out = keys.front().value;
        return true;
    }
    for (size_t i = 1; i < keys.size(); ++i) {
        if (time <= keys[i].time) {
            const K &a = keys[i - 1], &b = keys[i];
            float f = b.time > a.time ? float(time - a.time) / float(b.time - a.time) : 1.0f;
            out = a.interp ? lerp(a.value, b.value, f) : a.value;
            return true;
        }
    }
    out = keys.back().value;
    return true;
}

// Times are in frames; entity tracks drive their entity's transform.
void Movie::Animate(int time)
{
    auto vl = [](const math::vec3<float>& a, const math::vec3<float>& b, float f) { return math::lerp(a, b, f); };
    auto ql = [](const math::quat<float>& a, const math::quat<float>& b, float f) { return math::lerp(a, b, f); };
    for (TrackData& t : m_tracks) {
        if (!t.entity || !t.entity->IsMovieEnabled())
            continue;
        math::vec3<float> p = t.entity->GetPosition(), sc = t.entity->GetScale();
        math::quat<float> r = t.entity->GetRotation();
        bool a = Sample(t.position, time, p, vl), b = Sample(t.rotation, time, r, ql), c = Sample(t.scale, time, sc, vl);
        if (a || b || c)
            t.entity->SetTransform(p, r, sc);
    }
}

void Movie::Update(int dtMs)
{
    if (!m_playing || !m_msPerFrame)
        return;
    m_accumMs += dtMs;
    int t = m_time + m_accumMs / int(m_msPerFrame);
    m_accumMs %= int(m_msPerFrame);
    if (t > m_end) {
        if (m_loop)
            t = m_start + (t - m_start) % (m_end - m_start + 1);
        else {
            t = m_end;
            m_playing = false;
        }
    }
    m_time = t;
    Animate(m_time);
}

}
