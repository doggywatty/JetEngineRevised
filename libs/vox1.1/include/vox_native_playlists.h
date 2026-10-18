// libs/vox1.1/include/vox_native_playlists.h
// The playlists of a native ("VoxN", ".vxn") file: a playlist is the groups of segments one state of the music plays; a group
// is a list of elements (a segment played some times) that is read in order or at random. The records are those of the
// "Plst" (PlaylistInfos), "Grps" (GroupInfos) and "Grpe" (PlaylistElementInfos) chunks, see DecoderNativeCursor::ParseFile
// (@0x5cefac).
#ifndef LIBS_VOX1_1_INCLUDE_VOX_NATIVE_PLAYLISTS_H
#define LIBS_VOX1_1_INCLUDE_VOX_NATIVE_PLAYLISTS_H

#include <vector>

namespace vox {

// "Plst": the groupSwitchMode (0: the groups play one after the other) and the number of times the playlist is played
struct PlaylistInfos {
    int groupSwitchMode;
    int nbLoops;
};

// "Grps": the playlist the group is of, its index in it, how its elements are chosen (0 in order, 1 at random), the
// number of times it is played (-1 for ever) and the number of elements that a random group does not repeat
struct GroupInfos {
    int playlist;
    int group;
    int selectMode;
    int reserved;
    int nbLoops;
    int noRepeatLength;
};

// "Grpe": the playlist and the group of the element, its position in the group, its weight in a random group, the segment it
// plays, the entry point, whether the pre entry is played, the times the segment is played (-1 for ever), and the post exit
struct PlaylistElementInfos {
    int playlist;
    int position;
    int group;
    int segmentIndex;
    int entryPointType;
    int playPreEntry;
    int nbLoops;
    int playPostExit;
};

struct PlaylistElement {  // sizeof 0x14
    PlaylistElement() { Reset(); }
    void Reset() { m_segmentIndex = -1; m_entryPointType = 0; m_playPreEntry = 0; m_playPostExit = 0; m_nbLoops = 0; }
    int m_segmentIndex;  // 0x0
    int m_entryPointType;  // 0x4
    int m_playPreEntry;  // 0x8
    int m_playPostExit;  // 0xC
    int m_nbLoops;  // 0x10
};

class SegmentGroup {  // sizeof 0x24
public:
    SegmentGroup(const GroupInfos &infos) : m_selectMode(infos.selectMode), m_nbLoops(infos.nbLoops), m_nbLoopsRemaining(infos.nbLoops) {}
    virtual ~SegmentGroup() {}
    int GetSelectMode() const { return m_selectMode; }
    virtual void AddElement(const PlaylistElement &element, int weight) = 0;
    // the next element of the group, false when the group is over (all its loops were played)
    virtual bool NextElement(PlaylistElement &out) = 0;
    virtual void Reset() = 0;
protected:
    int m_selectMode;  // 0x8
    int m_nbLoops;  // 0xC
    int m_nbLoopsRemaining;  // 0x14
};

class SequentialGroup : public SegmentGroup {  // sizeof 0x40
public:
    SequentialGroup(const GroupInfos &infos) : SegmentGroup(infos), m_currentPosition(0) {}
    virtual void AddElement(const PlaylistElement &element, int weight);
    virtual bool NextElement(PlaylistElement &out);
    virtual void Reset();
private:
    std::vector<PlaylistElement> m_elements;  // 0x24
    int m_currentPosition;  // 0x38
};

class RandomGroup : public SegmentGroup {  // sizeof 0x74
public:
    RandomGroup(const GroupInfos &infos);
    virtual void AddElement(const PlaylistElement &element, int weight);
    virtual bool NextElement(PlaylistElement &out);
    virtual void Reset();
private:
    std::vector<PlaylistElement> m_elements;
    std::vector<int> m_weights;
    std::vector<int> m_recent;  // the last elements played, which are not drawn again (m_noRepeatLength of them)
    int m_noRepeatLength;  // 0x54
    int m_playbackCount;
    unsigned m_seed;
};

class NativePlaylist {  // sizeof 0x4C
public:
    NativePlaylist(const PlaylistInfos &infos);
    ~NativePlaylist();
    void AddGroup(const GroupInfos &infos);
    void AddPlaylistElement(const PlaylistElementInfos &infos);
    void Reset();
    // the next element of the playlist, false when it is over
    bool NextElement(PlaylistElement &out);
private:
    int m_groupSwitchMode;  // 0x4
    int m_nbLoops;  // 0x8
    int m_nbLoopsRemaining;  // 0x14
    int m_currentGroup;  // 0xC
    std::vector<SegmentGroup*> m_groups;  // 0x24
};

// The decoder owns the one made from the chunks; every cursor plays its own copy (the copy constructor of the .so's class)
class NativePlaylistsManager {  // sizeof 0x10
public:
    NativePlaylistsManager();
    NativePlaylistsManager(const NativePlaylistsManager &other);
    ~NativePlaylistsManager();
    void Init(int nbPlaylists);
    void AddPlaylist(int index, const PlaylistInfos &infos);
    void AddGroup(const GroupInfos &infos);
    void AddPlaylistElement(const PlaylistElementInfos &infos);
    int GetNbPlaylists() const { return (int)m_playlists.size(); }
    NativePlaylist *GetPlaylist(int index) { return index >= 0 && index < (int)m_playlists.size() ? m_playlists[index] : 0; }
    bool IsValid() const { return m_isValid; }
    void ResetPlaylist(int index);
private:
    void Clear();
    bool m_isValid;  // 0x0
    std::vector<NativePlaylist*> m_playlists;  // 0xC
    std::vector<PlaylistInfos> m_playlistInfos;
    std::vector<GroupInfos> m_groupInfos;
    std::vector<PlaylistElementInfos> m_elementInfos;
};

}  // namespace vox

#endif
