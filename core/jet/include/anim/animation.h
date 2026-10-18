// core/jet/include/anim/animation.h
#ifndef CORE_JET_INCLUDE_ANIM_ANIMATION_H
#define CORE_JET_INCLUDE_ANIM_ANIMATION_H

#include <map>
#include <vector>
#include <boost/shared_array.hpp>
#include "smath.h"
#include "core/pstring.h"
#include "res/resource.h"

namespace jet::anim {

// A keyframe animation (.anim, jet::anim::Animation in the 0.0.3 .so; InitV100 @0x721824,
// Load @0x721a34). Little endian, strings u16 length + bytes:
//   u32 fps
//   s16 node count; per node: str name, u8 flags (bit 1: compressed rotations)
//   s16 material count; per material: str name, s16 texture channel count
//   position channel of every node, then rotation channel of every node, then scale channel
//   of every node; per material: its channels' uv position, then uv rotation, then uv scale
//   u8 (0)
// A channel is u16 key count; with keys, u16 times (units of 8 ms, bit 15: hold the key, no
// interpolation to the next) then the keys, else a single value. Keys: position / scale float3,
// rotation 4 s16 (x y z w / 32767) or compressed 3 s16 (x y z / 32767, w = sqrt(1 - |xyz|^2)),
// uv position / scale float2, uv rotation float. The duration is the latest key time.
class Animation : public jet::res::Resource {
public:
    template <class K> struct KeyFrameSequence {
        boost::shared_array<unsigned short> times;
        boost::shared_array<K> keys;
    };
    struct PosKeyFrame { math::vec3<float> v; };
    struct ScaleKeyFrame { math::vec3<float> v; };
    struct RotKeyFrame { short v[4]; };
    struct CompressedRotKeyFrame { short v[3]; };
    struct UVPosKeyFrame { math::vec2<float> v; };
    struct UVRotKeyFrame { float v; };
    struct UVScaleKeyFrame { math::vec2<float> v; };

    // NodeChannel (0x2c)
    struct NodeChannel {
        jet::String name;
        unsigned char flags;
        unsigned short posCount;
        boost::shared_ptr<KeyFrameSequence<PosKeyFrame> > pos;
        math::vec3<float> initPos;
        unsigned short rotCount;
        boost::shared_ptr<KeyFrameSequence<RotKeyFrame> > rot;
        boost::shared_ptr<KeyFrameSequence<CompressedRotKeyFrame> > crot;
        math::quat<float> initRot;
        unsigned short scaleCount;
        boost::shared_ptr<KeyFrameSequence<ScaleKeyFrame> > scale;
        math::vec3<float> initScale;
    };
    // MaterialTexChannel (0x24)
    struct MaterialTexChannel {
        unsigned short posCount, rotCount, scaleCount;
        boost::shared_ptr<KeyFrameSequence<UVPosKeyFrame> > pos;
        boost::shared_ptr<KeyFrameSequence<UVRotKeyFrame> > rot;
        boost::shared_ptr<KeyFrameSequence<UVScaleKeyFrame> > scale;
        math::vec2<float> initPos;
        float initRot;
        math::vec2<float> initScale;
    };
    struct MaterialChannel {
        jet::String name;
        std::vector<MaterialTexChannel> channels;
    };

    static const unsigned k_flagCompressed = 2;

    Animation();
    virtual ~Animation();
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();
    virtual void Unload();

    unsigned GetFPS() const { return m_fps; }
    unsigned GetTPF() const { return m_fps ? 1000 / m_fps : 0; }
    unsigned GetDuration() const { return m_duration; }
    unsigned GetNodeCount() const { return (unsigned)m_nodeChannels.size(); }
    const jet::String &GetNodeName(unsigned i) const { return m_nodeChannels[i].name; }
    int FindNodeIdxByName(const jet::String&) const;
    unsigned GetMaterialCount() const { return (unsigned)m_materialChannels.size(); }
    unsigned GetMaterialTexChannelCount(unsigned m) const { return (unsigned)m_materialChannels[m].channels.size(); }
    const jet::String &GetMaterialName(unsigned i) const { return m_materialChannels[i].name; }
    int FindMaterialIdxByName(const jet::String&) const;
    unsigned char GetFlags(unsigned node) const { return m_nodeChannels[node].flags; }
    bool GetFlag(unsigned node, unsigned flag) const { return (m_nodeChannels[node].flags & flag) != 0; }

    bool HasNodePosChannel(unsigned n) const { return m_nodeChannels[n].posCount != 0; }
    bool HasNodeRotChannel(unsigned n) const { return m_nodeChannels[n].rotCount != 0; }
    bool HasNodeScaleChannel(unsigned n) const { return m_nodeChannels[n].scaleCount != 0; }
    const math::vec3<float> &GetNodeInitPos(unsigned n) const { return m_nodeChannels[n].initPos; }
    const math::quat<float> &GetNodeInitRot(unsigned n) const { return m_nodeChannels[n].initRot; }
    const math::vec3<float> &GetNodeInitScale(unsigned n) const { return m_nodeChannels[n].initScale; }

    // time in ms; the cache is the index of the last key found (a search hint)
    bool SampleNodePos(math::vec3<float>&, unsigned node, unsigned time, int *cache = 0);
    bool SampleNodeRot(math::quat<float>&, unsigned node, unsigned time, int *cache = 0);
    bool SampleNodeScale(math::vec3<float>&, unsigned node, unsigned time, int *cache = 0);
    bool SampleMaterialPos(math::vec2<float>&, unsigned mat, unsigned ch, unsigned time);
    bool SampleMaterialRot(float&, unsigned mat, unsigned ch, unsigned time);
    bool SampleMaterialScale(math::vec2<float>&, unsigned mat, unsigned ch, unsigned time);

private:
    bool InitV100(jet::stream::IStream&);
    void LoadIfNeeded();
    unsigned m_fps;
    unsigned m_duration;
    std::vector<NodeChannel> m_nodeChannels;
    std::map<jet::String, unsigned> m_nodeNameIdxMap;
    std::vector<MaterialChannel> m_materialChannels;
};

}

#endif
