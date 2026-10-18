// libs/manhattan/include/dlc/states.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_STATES_H
#define LIBS_MANHATTAN_INCLUDE_DLC_STATES_H

namespace vox {
struct States {  // sizeof 0x8
    States();
    int m_nbStates;  // 0x0
    int* m_pPlaylistIndexes;  // 0x4
};
}

#endif
