// source/game/actors/statemachine/StateDatabase.cpp
#include "StateDatabase.h"
#include "stream/streammgr.h"
#include "stream/istream.h"
#include <cstring>

StateDatabase *StateDatabase::GetInstance() {
    static StateDatabase s_instance;
    return &s_instance;
}

int StateData::GetParam(const char *n, int def) const {
    for (unsigned i = 0; i < params.size(); i++)
        if (params[i].name.Equals(n)) return params[i].value;
    return def;
}
const StateTrack *StateData::FindTrack(unsigned anim, const char *n) const {
    if (anim >= anims.size()) return 0;
    for (unsigned i = 0; i < anims[anim].tracks.size(); i++)
        if (anims[anim].tracks[i].name.Equals(n)) return &anims[anim].tracks[i];
    return 0;
}
float StateData::GetParamFloat(const char *n, float def) const {
    int v = GetParam(n, -1);
    if (v == -1) return def;
    return (float)((v << 5) >> 5) / (float)(1 << ((v >> 27) & 0xff));
}
int StateData::FindTrackIdxByName(unsigned anim, const char *n) const {
    if (anim >= anims.size()) return -1;
    for (unsigned i = 0; i < anims[anim].tracks.size(); i++)
        if (anims[anim].tracks[i].name.Equals(n)) return (int)i;
    return -1;
}
// @0x58b558: the state names the triggers and the selector lead to become indices
void StateData::Init(const StateSetData &set) {
    for (StateTrigger &t : triggers) {
        t.target = t.hasTarget ? set.FindStateIdxByName(t.targetName) : -1;
        t.targetName = jet::String::null;
    }
    for (SelectorEntryData &e : entries) e.state = set.FindStateIdxByName(e.name);
}
int StateSetData::FindStateIdxByName(const jet::String &n) const {
    for (unsigned i = 0; i < states.size(); i++)
        if (states[i].name.Equals(n)) return (int)i;
    return -1;
}

static void Read(jet::stream::IStream &s, StateTrigger &t) {
    s >> t.code >> t.kind >> t.param >> t.extra >> t.start >> t.delay;
    t.end = -1;
    s >> t.hasTarget;
    t.target = -1;
    if (t.hasTarget) s >> t.targetName;
    unsigned short n = 0;
    s >> n;
    t.conditions.resize(n);
    for (unsigned i = 0; i < n; i++) s >> t.conditions[i].name >> t.conditions[i].value;
}

static void Read(jet::stream::IStream &s, StateTrack &t) {
    s >> t.name;
    unsigned short n = 0;
    s >> n;
    t.events.resize(n);
    for (unsigned i = 0; i < n; i++) {
        StateEvent &e = t.events[i];
        int count = 0;
        s >> e.time >> e.duration >> count;
        e.args.resize(count > 0 ? count : 0);
        for (int k = 0; k < count; k++) s >> e.args[k];
        bool named = false;
        s >> named;
        if (named) s >> e.name;
    }
    s >> t.end;
}

static void Read(jet::stream::IStream &s, StateData &d) {
    s >> d.name;
    s >> d.isSelector;
    unsigned short n = 0;
    if (d.isSelector) {
        s >> n;
        d.entries.resize(n);
        for (unsigned i = 0; i < n; i++) {
            SelectorEntryData &e = d.entries[i];
            s >> e.name >> e.weight;
            s.Read(&e.factor, 4);
            s >> e.threshold;
            e.state = -1;
        }
    }
    s >> n;
    d.params.resize(n);
    for (unsigned i = 0; i < n; i++) s >> d.params[i].name >> d.params[i].value;
    s >> n;
    d.triggers.resize(n);
    for (unsigned i = 0; i < n; i++) Read(s, d.triggers[i]);
    if (d.isSelector) return;
    s >> n;
    d.anims.resize(n);
    for (unsigned i = 0; i < n; i++) {
        StateAnim &a = d.anims[i];
        s >> a.name >> a.set;
        unsigned short tracks = 0;
        s >> tracks;
        a.tracks.resize(tracks);
        for (unsigned k = 0; k < tracks; k++) Read(s, a.tracks[k]);
    }
    s >> n;
    d.animSets.resize(n);
    for (unsigned i = 0; i < n; i++) s >> d.animSets[i].name >> d.animSets[i].first >> d.animSets[i].count;
}

bool StateDatabase::Load(const jet::String &name) {
    jet::stream::IStream *sp = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!sp) return false;
    jet::stream::IStream &s = *sp;
    s.Start();
    unsigned short sets = 0;
    s >> sets;
    m_sets.resize(sets);
    for (unsigned i = 0; i < sets; i++) {
        StateSetData &set = m_sets[i];
        s >> set.name;
        unsigned short params = 0;
        s >> params;
        for (unsigned k = 0; k < params; k++) {
            int v = 0;
            jet::String skipped;
            s >> v >> skipped;
        }
        unsigned short states = 0;
        s >> states;
        set.states.resize(states);
        for (unsigned k = 0; k < states; k++) Read(s, set.states[k]);
        for (unsigned k = 0; k < states; k++) set.states[k].Init(set);
    }
    bool ok = s.Tell() == s.Size();
    s.Finish();
    delete sp;
    return ok;
}

const StateSetData *StateDatabase::FindStateSetDataByName(const jet::String &n) const {
    for (unsigned i = 0; i < m_sets.size(); i++)
        if (m_sets[i].name.Equals(n)) return &m_sets[i];
    return 0;
}

bool StateEvent::IsOnTime(int prev, int cur, int loopStart, int loopEnd) const {
    const int end = time + (duration < 0 ? -duration : duration);
    if (prev <= cur) return time <= cur && prev < end;
    if (time <= cur ? loopStart < end : false) return true;
    return prev <= end ? time <= loopEnd : false;
}

int StateTrack::FindEventIdxByTime(int prev, int cur, int loopStart, int loopEnd, int nth) const {
    int found = 0;
    for (unsigned i = 0; i < events.size(); i++) {
        const int start = events[i].time, end = start + (events[i].duration < 0 ? -events[i].duration : events[i].duration);
        bool on;
        if (cur < prev) {
            on = (start <= cur && loopStart < end) || (prev <= end && start <= loopEnd);
            if (!on) continue;
        } else {
            on = start <= cur && prev < end;
            if (!on) continue;
        }
        if (found++ == nth) return (int)i;
    }
    return -1;
}

const StateEvent *StateTrack::FindEventByTime(int prev, int cur, int loopStart, int loopEnd) const {
    int i = FindEventIdxByTime(prev, cur, loopStart, loopEnd);
    return i < 0 ? 0 : &events[(unsigned)i];
}

const StateEvent *StateTrack::FindLastEvent() const {
    const StateEvent *last = 0;
    for (const StateEvent &e : events)
        if (!last || last->time + last->duration < e.time + e.duration) last = &e;
    return last;
}
