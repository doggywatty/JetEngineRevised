// libs/vox1.1/src/vox_native_playlists.cpp
#include "vox_native_playlists.h"

namespace vox {

void SequentialGroup::AddElement(const PlaylistElement &element, int) { m_elements.push_back(element); }

void SequentialGroup::Reset() {
    m_currentPosition = 0;
    m_nbLoopsRemaining = m_nbLoops;
}

bool SequentialGroup::NextElement(PlaylistElement &out) {
    if (m_elements.empty()) return false;
    if (m_currentPosition >= (int)m_elements.size()) {
        // the group has been played once: again while it has loops (a negative number of them is for ever)
        if (m_nbLoopsRemaining >= 0 && --m_nbLoopsRemaining <= 0) return false;
        m_currentPosition = 0;
    }
    out = m_elements[m_currentPosition++];
    return true;
}

RandomGroup::RandomGroup(const GroupInfos &infos)
    : SegmentGroup(infos), m_noRepeatLength(infos.noRepeatLength < 0 ? 0 : infos.noRepeatLength), m_playbackCount(0), m_seed(2463534242u) {}

void RandomGroup::AddElement(const PlaylistElement &element, int weight) {
    m_elements.push_back(element);
    m_weights.push_back(weight > 0 ? weight : 1);
}

void RandomGroup::Reset() {
    m_playbackCount = 0;
    m_recent.clear();
    m_nbLoopsRemaining = m_nbLoops;
}

// a draw by the weights of the elements that were not played in the last m_noRepeatLength draws
bool RandomGroup::NextElement(PlaylistElement &out) {
    const int n = (int)m_elements.size();
    if (!n) return false;
    if (m_nbLoops >= 0 && m_playbackCount >= m_nbLoops * n) return false;
    int noRepeat = m_noRepeatLength < n ? m_noRepeatLength : n - 1;
    int sum = 0;
    for (int i = 0; i < n; i++) {
        bool recent = false;
        for (int r : m_recent) recent = recent || r == i;
        if (!recent) sum += m_weights[i];
    }
    m_seed ^= m_seed << 13; m_seed ^= m_seed >> 17; m_seed ^= m_seed << 5;
    int pick = (int)(m_seed % (unsigned)sum), chosen = 0;
    for (int i = 0; i < n; i++) {
        bool recent = false;
        for (int r : m_recent) recent = recent || r == i;
        if (recent) continue;
        if (pick < m_weights[i]) { chosen = i; break; }
        pick -= m_weights[i];
    }
    m_recent.push_back(chosen);
    if ((int)m_recent.size() > noRepeat) m_recent.erase(m_recent.begin());
    m_playbackCount++;
    out = m_elements[chosen];
    return true;
}

NativePlaylist::NativePlaylist(const PlaylistInfos &infos)
    : m_groupSwitchMode(infos.groupSwitchMode), m_nbLoops(infos.nbLoops), m_nbLoopsRemaining(infos.nbLoops), m_currentGroup(0) {}

NativePlaylist::~NativePlaylist() {
    for (SegmentGroup *g : m_groups) delete g;
}

void NativePlaylist::AddGroup(const GroupInfos &infos) {
    if (infos.selectMode == 1) m_groups.push_back(new RandomGroup(infos));
    else m_groups.push_back(new SequentialGroup(infos));
}

void NativePlaylist::AddPlaylistElement(const PlaylistElementInfos &infos) {
    if (infos.group < 0 || infos.group >= (int)m_groups.size()) return;
    PlaylistElement e;
    e.m_segmentIndex = infos.segmentIndex;
    e.m_entryPointType = infos.entryPointType;
    e.m_playPreEntry = infos.playPreEntry;
    e.m_playPostExit = infos.playPostExit;
    e.m_nbLoops = infos.nbLoops;
    // none of the files of the game has a random group, so the weights of its elements (not in the record) are all equal
    m_groups[infos.group]->AddElement(e, 1);
}

void NativePlaylist::Reset() {
    m_currentGroup = 0;
    m_nbLoopsRemaining = m_nbLoops;
    for (SegmentGroup *g : m_groups) g->Reset();
}

bool NativePlaylist::NextElement(PlaylistElement &out) {
    for (;;) {
        while (m_currentGroup < (int)m_groups.size()) {
            if (m_groups[m_currentGroup]->NextElement(out)) return true;
            m_currentGroup++;
        }
        // all the groups were played: the playlist again while it has loops
        if (m_nbLoopsRemaining >= 0 && --m_nbLoopsRemaining <= 0) return false;
        const int loops = m_nbLoopsRemaining;
        Reset();
        m_nbLoopsRemaining = loops;
        if (m_groups.empty()) return false;
    }
}

NativePlaylistsManager::NativePlaylistsManager() : m_isValid(true) {}

NativePlaylistsManager::NativePlaylistsManager(const NativePlaylistsManager &other) : m_isValid(other.m_isValid) {
    Init((int)other.m_playlists.size());
    for (size_t i = 0; i < other.m_playlistInfos.size(); i++) AddPlaylist((int)i, other.m_playlistInfos[i]);
    for (const GroupInfos &g : other.m_groupInfos) AddGroup(g);
    for (const PlaylistElementInfos &e : other.m_elementInfos) AddPlaylistElement(e);
}

NativePlaylistsManager::~NativePlaylistsManager() { Clear(); }

void NativePlaylistsManager::Clear() {
    for (NativePlaylist *p : m_playlists) delete p;
    m_playlists.clear();
}

void NativePlaylistsManager::Init(int nbPlaylists) {
    Clear();
    m_playlists.assign(nbPlaylists > 0 ? nbPlaylists : 0, (NativePlaylist*)0);
    m_playlistInfos.assign(m_playlists.size(), PlaylistInfos());
}

void NativePlaylistsManager::AddPlaylist(int index, const PlaylistInfos &infos) {
    if (index < 0 || index >= (int)m_playlists.size()) { m_isValid = false; return; }
    delete m_playlists[index];
    m_playlists[index] = new NativePlaylist(infos);
    m_playlistInfos[index] = infos;
}

void NativePlaylistsManager::AddGroup(const GroupInfos &infos) {
    NativePlaylist *p = GetPlaylist(infos.playlist);
    if (!p) { m_isValid = false; return; }
    p->AddGroup(infos);
    m_groupInfos.push_back(infos);
}

void NativePlaylistsManager::AddPlaylistElement(const PlaylistElementInfos &infos) {
    NativePlaylist *p = GetPlaylist(infos.playlist);
    if (!p) { m_isValid = false; return; }
    p->AddPlaylistElement(infos);
    m_elementInfos.push_back(infos);
}

void NativePlaylistsManager::ResetPlaylist(int index) {
    if (NativePlaylist *p = GetPlaylist(index)) p->Reset();
}

}  // namespace vox
