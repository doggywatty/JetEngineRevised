// Prints a state set of the states datalib: its states (name, type, label) and their triggers.
//   test_states <data dir> [set name filter]
#include "game/actors/statemachine/StateDatabase.h"
#include "stream/streammgr.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc > 1 && chdir(argv[1])) return 1;
    // as the game does (Game.cpp): each datalib folder is a stream root, and its files are named without the folder
    jet::stream::StreamMgr::GetInstance().AddPath(jet::String("datalibs"), jet::String("*"));
    StateDatabase *db = StateDatabase::GetInstance();
    if (!db->Load(jet::String("states"))) {
        printf("no states\n");
        return 1;
    }
    for (unsigned i = 0; i < db->GetStateSetCount(); i++) {
        const StateSetData *set = db->GetStateSet(i);
        if (argc > 2 && !strstr(set->name.c_str(), argv[2])) continue;
        printf("SET %s\n", set->name.c_str());
        for (unsigned s = 0; s < set->states.size(); s++) {
            const StateData &d = set->states[s];
            printf("  %u %s type=%d label=%d%s anim=%s\n", s, d.name.c_str(), d.GetParam("StateType", 0), d.GetParam("StateLabel", 1), d.isSelector ? " selector" : "",
                   d.anims.empty() ? "" : d.anims[0].name.c_str());
            for (const StateTrigger &t : d.triggers)
                printf("      trig kind=%d code=%d param=%d -> %s (%d) start=%d end=%d delay=%d\n", t.kind, t.code, t.param, t.targetName.c_str(), t.target, t.start, t.end, t.delay);
        }
    }
    return 0;
}
