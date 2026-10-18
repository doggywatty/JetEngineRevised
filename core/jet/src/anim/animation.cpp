// core/jet/src/anim/animation.cpp
#include "anim/animation.h"
#include "stream/istream.h"
#include <cmath>

namespace jet::anim {

Animation::Animation() : m_fps(30), m_duration(0) {}
Animation::~Animation() { Unload(); }

bool Animation::Init(jet::stream::IStream *s) {
    if (!s) return false;
    SetStream(s);
    s->Start();
    s->SeekBeg(0);
    bool ok = InitV100(*s);
    s->Finish();
    return ok;
}

// InitV100 @0x721824: the header only (names, so the animation can be bound before loading)
bool Animation::InitV100(jet::stream::IStream &s) {
    s >> m_fps;
    short nodes = 0;
    s >> nodes;
    m_nodeChannels.clear();
    m_nodeChannels.resize(nodes < 0 ? 0 : nodes);
    m_nodeNameIdxMap.clear();
    for (short i = 0; i < nodes; i++) {
        s >> m_nodeChannels[i].name;
        s >> m_nodeChannels[i].flags;
        m_nodeNameIdxMap.insert(std::make_pair(m_nodeChannels[i].name, (unsigned)i));
    }
    short mats = 0;
    s >> mats;
    m_materialChannels.clear();
    m_materialChannels.resize(mats < 0 ? 0 : mats);
    for (short i = 0; i < mats; i++) {
        s >> m_materialChannels[i].name;
        short ch = 0;
        s >> ch;
        m_materialChannels[i].channels.resize(ch < 0 ? 0 : ch);
    }
    return true;
}

template <class K>
static boost::shared_ptr<Animation::KeyFrameSequence<K> > ReadKeys(jet::stream::IStream &s, unsigned short count, unsigned &duration) {
    boost::shared_ptr<Animation::KeyFrameSequence<K> > seq(new Animation::KeyFrameSequence<K>());
    seq->times.reset(new unsigned short[count]);
    s.Read(seq->times.get(), count * 2);
    unsigned last = (seq->times[count - 1] & 0x7fffu) << 3;
    if (last > duration) duration = last;
    seq->keys.reset(new K[count]);
    s.Read(seq->keys.get(), count * sizeof(K));
    return seq;
}

static math::quat<float> DecompressRotation(const short *v) {
    const float k = 1.0f / 32767.0f;
    float x = v[0] * k, y = v[1] * k, z = v[2] * k;
    float w2 = 1.0f - z * z - x * x - y * y;
    math::quat<float> q(x, y, z, sqrtf(w2 < 0 ? 0 : w2));
    q.normalize();
    return q;
}
static math::quat<float> ExpandRotation(const short *v) {
    const float k = 1.0f / 32767.0f;
    return math::quat<float>(v[0] * k, v[1] * k, v[2] * k, v[3] * k);
}

// Load @0x721a34
bool Animation::Load() {
    jet::stream::IStream *sp = GetStream();
    if (!sp) return false;
    jet::stream::IStream &s = *sp;
    s.Start();
    s.SeekBeg(0);
    InitV100(s);
    unsigned duration = 0;
    for (unsigned i = 0; i < m_nodeChannels.size(); i++) {
        NodeChannel &n = m_nodeChannels[i];
        s >> n.posCount;
        n.initPos = math::vec3<float>(0, 0, 0);
        if (n.posCount) n.pos = ReadKeys<PosKeyFrame>(s, n.posCount, duration);
        else s.Read(&n.initPos, 12);
    }
    for (unsigned i = 0; i < m_nodeChannels.size(); i++) {
        NodeChannel &n = m_nodeChannels[i];
        const bool compressed = (n.flags & k_flagCompressed) != 0;
        s >> n.rotCount;
        n.initRot = math::quat<float>(0, 0, 0, 1);
        if (n.rotCount) {
            if (compressed) n.crot = ReadKeys<CompressedRotKeyFrame>(s, n.rotCount, duration);
            else n.rot = ReadKeys<RotKeyFrame>(s, n.rotCount, duration);
        } else if (compressed) {
            short v[3];
            s.Read(v, 6);
            n.initRot = DecompressRotation(v);
        } else {
            short v[4];
            s.Read(v, 8);
            n.initRot = ExpandRotation(v);
        }
    }
    for (unsigned i = 0; i < m_nodeChannels.size(); i++) {
        NodeChannel &n = m_nodeChannels[i];
        s >> n.scaleCount;
        n.initScale = math::vec3<float>(0, 0, 0);
        if (n.scaleCount) n.scale = ReadKeys<ScaleKeyFrame>(s, n.scaleCount, duration);
        else s.Read(&n.initScale, 12);
    }
    for (unsigned m = 0; m < m_materialChannels.size(); m++) {
        std::vector<MaterialTexChannel> &chs = m_materialChannels[m].channels;
        for (unsigned c = 0; c < chs.size(); c++) {
            s >> chs[c].posCount;
            chs[c].initPos = math::vec2<float>(0, 0);
            if (chs[c].posCount) chs[c].pos = ReadKeys<UVPosKeyFrame>(s, chs[c].posCount, duration);
            else s.Read(&chs[c].initPos, 8);
        }
        for (unsigned c = 0; c < chs.size(); c++) {
            s >> chs[c].rotCount;
            chs[c].initRot = 0;
            if (chs[c].rotCount) chs[c].rot = ReadKeys<UVRotKeyFrame>(s, chs[c].rotCount, duration);
            else s.Read(&chs[c].initRot, 4);
        }
        for (unsigned c = 0; c < chs.size(); c++) {
            s >> chs[c].scaleCount;
            chs[c].initScale = math::vec2<float>(0, 0);
            if (chs[c].scaleCount) chs[c].scale = ReadKeys<UVScaleKeyFrame>(s, chs[c].scaleCount, duration);
            else s.Read(&chs[c].initScale, 8);
        }
    }
    unsigned char tail = 0;
    s >> tail;
    s.Finish();
    m_duration = duration;
    SetLoaded(true);
    return true;
}

void Animation::Unload() {
    for (unsigned i = 0; i < m_nodeChannels.size(); i++) {
        m_nodeChannels[i].pos.reset();
        m_nodeChannels[i].rot.reset();
        m_nodeChannels[i].crot.reset();
        m_nodeChannels[i].scale.reset();
    }
    SetLoaded(false);
}

void Animation::LoadIfNeeded() {
    if (!IsLoaded()) Load();
}

int Animation::FindNodeIdxByName(const jet::String &n) const {
    std::map<jet::String, unsigned>::const_iterator it = m_nodeNameIdxMap.find(n);
    return it == m_nodeNameIdxMap.end() ? -1 : (int)it->second;
}
int Animation::FindMaterialIdxByName(const jet::String &n) const {
    for (unsigned i = 0; i < m_materialChannels.size(); i++)
        if (m_materialChannels[i].name.Equals(n)) return (int)i;
    return -1;
}

// AnimationUtil<K>::Sample* (@0x72cd54..): the key at or before t (t / 8 against the key
// times, searched from the cached key), interpolated to the next one unless it holds.
// Returns the key index and the blend factor (-1 when no blend).
static unsigned FindKey(const unsigned short *times, unsigned count, unsigned time, int *cache, float &f) {
    unsigned t = time >> 3;
    unsigned lo = 0, hi = count;
    if (cache && *cache >= 0 && (unsigned)*cache < count && t >= (times[*cache] & 0x7fffu)) lo = (unsigned)*cache;
    // binary search for the last key with time <= t
    while (lo + 1 < hi) {
        unsigned mid = lo + (hi - lo) / 2;
        if (t < (times[mid] & 0x7fffu)) hi = mid;
        else lo = mid;
    }
    if (cache) *cache = (int)lo;
    f = -1;
    if (lo + 1 < count && !(times[lo] & 0x8000u)) {
        unsigned t0 = (times[lo] & 0x7fffu) << 3, t1 = (times[lo + 1] & 0x7fffu) << 3;
        if (t1 > t0) {
            f = (float)(time > t0 ? time - t0 : 0) / (float)(t1 - t0);
            if (f > 1) f = 1;
        }
    }
    return lo;
}

bool Animation::SampleNodePos(math::vec3<float> &out, unsigned node, unsigned time, int *cache) {
    LoadIfNeeded();
    const NodeChannel &n = m_nodeChannels[node];
    if (!n.posCount) { out = n.initPos; return true; }
    float f;
    unsigned k = FindKey(n.pos->times.get(), n.posCount, time, cache, f);
    const math::vec3<float> &a = n.pos->keys[k].v;
    if (f < 0) { out = a; return true; }
    const math::vec3<float> &b = n.pos->keys[k + 1].v;
    out = math::vec3<float>(a.x + (b.x - a.x) * f, a.y + (b.y - a.y) * f, a.z + (b.z - a.z) * f);
    return true;
}

bool Animation::SampleNodeScale(math::vec3<float> &out, unsigned node, unsigned time, int *cache) {
    LoadIfNeeded();
    const NodeChannel &n = m_nodeChannels[node];
    if (!n.scaleCount) { out = n.initScale; return true; }
    float f;
    unsigned k = FindKey(n.scale->times.get(), n.scaleCount, time, cache, f);
    const math::vec3<float> &a = n.scale->keys[k].v;
    if (f < 0) { out = a; return true; }
    const math::vec3<float> &b = n.scale->keys[k + 1].v;
    out = math::vec3<float>(a.x + (b.x - a.x) * f, a.y + (b.y - a.y) * f, a.z + (b.z - a.z) * f);
    return true;
}

bool Animation::SampleNodeRot(math::quat<float> &out, unsigned node, unsigned time, int *cache) {
    LoadIfNeeded();
    const NodeChannel &n = m_nodeChannels[node];
    if (!n.rotCount) { out = n.initRot; return true; }
    float f;
    const bool compressed = (n.flags & k_flagCompressed) != 0;
    unsigned k = FindKey(compressed ? n.crot->times.get() : n.rot->times.get(), n.rotCount, time, cache, f);
    math::quat<float> a = compressed ? DecompressRotation(n.crot->keys[k].v) : ExpandRotation(n.rot->keys[k].v);
    if (f < 0) { out = a; return true; }
    math::quat<float> b = compressed ? DecompressRotation(n.crot->keys[k + 1].v) : ExpandRotation(n.rot->keys[k + 1].v);
    out = math::nlerp(a, b, f);
    return true;
}

template <class K, class V>
static V SampleUV(const boost::shared_ptr<Animation::KeyFrameSequence<K> > &seq, unsigned count, const V &init, unsigned time) {
    if (!count) return init;
    float f;
    unsigned k = FindKey(seq->times.get(), count, time, 0, f);
    if (f < 0) return seq->keys[k].v;
    return seq->keys[k].v + (seq->keys[k + 1].v - seq->keys[k].v) * f;
}

bool Animation::SampleMaterialPos(math::vec2<float> &out, unsigned m, unsigned c, unsigned time) {
    LoadIfNeeded();
    const MaterialTexChannel &ch = m_materialChannels[m].channels[c];
    out = SampleUV(ch.pos, ch.posCount, ch.initPos, time);
    return true;
}
bool Animation::SampleMaterialRot(float &out, unsigned m, unsigned c, unsigned time) {
    LoadIfNeeded();
    const MaterialTexChannel &ch = m_materialChannels[m].channels[c];
    out = SampleUV(ch.rot, ch.rotCount, ch.initRot, time);
    return true;
}
bool Animation::SampleMaterialScale(math::vec2<float> &out, unsigned m, unsigned c, unsigned time) {
    LoadIfNeeded();
    const MaterialTexChannel &ch = m_materialChannels[m].channels[c];
    out = SampleUV(ch.scale, ch.scaleCount, ch.initScale, time);
    return true;
}

}
