// libs/vox1.1/src/vox_group.cpp
#include <cctype>
#include "vox_internal.h"

namespace vox {

static bool NameEquals(const std::string &a, const char *b) {
    size_t i = 0;
    for (; i < a.size() && b[i]; i++)
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
    return i == a.size() && !b[i];
}

// the id of a group is its place in the list
unsigned GroupManager::AddGroup(const group::CreationSettings &s) {
    Group g;
    g.name = s.m_name ? s.m_name : "";
    g.bus = s.m_busName ? s.m_busName : "";
    g.settings = s;
    g.settings.m_name = 0;
    g.settings.m_busName = 0;
    g.volume = Fader(s.m_volume, s.m_volume, 0);
    g.pitch = Fader(s.m_pitch, s.m_pitch, 0);
    g.enable = Fader(s.m_enable ? 1.0f : 0.0f, s.m_enable ? 1.0f : 0.0f, 0);
    m_groups.push_back(g);
    return (unsigned)m_groups.size() - 1;
}

bool GroupManager::ReconfigureGroup(unsigned id, const group::CreationSettings &s) {
    if (id >= m_groups.size()) return false;
    Group &g = m_groups[id];
    if (s.m_name) g.name = s.m_name;
    if (s.m_busName) g.bus = s.m_busName;
    g.settings = s;
    g.settings.m_name = 0;
    g.settings.m_busName = 0;
    g.volume = Fader(s.m_volume, s.m_volume, 0);
    g.pitch = Fader(s.m_pitch, s.m_pitch, 0);
    g.enable = Fader(s.m_enable ? 1.0f : 0.0f, s.m_enable ? 1.0f : 0.0f, 0);
    return true;
}

unsigned GroupManager::GetGroupId(const char *name) const {
    if (!name) return (unsigned)-1;
    for (size_t i = 0; i < m_groups.size(); i++)
        if (NameEquals(m_groups[i].name, name)) return (unsigned)i;
    return (unsigned)-1;
}

bool GroupManager::GetGroupName(unsigned id, char *out, int size) const {
    if (id >= m_groups.size() || size <= 0) return false;
    strncpy(out, m_groups[id].name.c_str(), size - 1);
    out[size - 1] = 0;
    return true;
}

// is `ancestor` the group, or one of the groups it is in
bool GroupManager::IsChild(unsigned child, unsigned ancestor) const {
    for (int depth = 0; depth < 32 && child < m_groups.size(); depth++) {
        if (child == ancestor) return true;
        child = m_groups[child].settings.m_parentId;
    }
    return false;
}

bool GroupManager::GetEnable(unsigned id) const { return id < m_groups.size() && m_groups[id].enable.GetEndValue() > 0.5f; }

bool GroupManager::SetEnable(unsigned id, bool enable, float fade) {
    if (id >= m_groups.size()) return false;
    Group &g = m_groups[id];
    g.enable = Fader(g.enable.GetCurrentValue(), enable ? 1.0f : 0.0f, fade);
    return true;
}

float GroupManager::GetVolume(unsigned id) const { return id < m_groups.size() ? m_groups[id].volume.GetEndValue() : 0.0f; }

bool GroupManager::SetVolume(unsigned id, float volume, float fade) {
    if (id >= m_groups.size()) return false;
    Group &g = m_groups[id];
    g.volume = Fader(g.volume.GetCurrentValue(), volume, fade);
    return true;
}

float GroupManager::GetPitch(unsigned id) const { return id < m_groups.size() ? m_groups[id].pitch.GetEndValue() : 1.0f; }

bool GroupManager::SetPitch(unsigned id, float pitch, float fade) {
    if (id >= m_groups.size()) return false;
    Group &g = m_groups[id];
    g.pitch = Fader(g.pitch.GetCurrentValue(), pitch, fade);
    return true;
}

void GroupManager::Update(float dt) {
    for (size_t i = 0; i < m_groups.size(); i++) {
        m_groups[i].volume.Update(dt);
        m_groups[i].pitch.Update(dt);
        m_groups[i].enable.Update(dt);
    }
}

float GroupManager::GetEffectiveVolume(unsigned id) const {
    float v = 1.0f;
    for (int depth = 0; depth < 32 && id < m_groups.size(); depth++) {
        const Group &g = m_groups[id];
        v *= g.volume.GetCurrentValue() * g.enable.GetCurrentValue();
        id = g.settings.m_parentId;
    }
    return v;
}

float GroupManager::GetEffectivePitch(unsigned id) const {
    float p = 1.0f;
    for (int depth = 0; depth < 32 && id < m_groups.size(); depth++) {
        p *= m_groups[id].pitch.GetCurrentValue();
        id = m_groups[id].settings.m_parentId;
    }
    return p;
}

}  // namespace vox
