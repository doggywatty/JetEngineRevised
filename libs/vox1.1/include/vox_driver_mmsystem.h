// libs/vox1.1/include/vox_driver_mmsystem.h
// The sound device of Windows: waveOut with a ring of blocks. (The microphone callbacks of the .so's class are not here: the game
// does not record.)
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DRIVER_MMSYSTEM_H
#define LIBS_VOX1_1_INCLUDE_VOX_DRIVER_MMSYSTEM_H

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include "vox_driver_pumped.h"

namespace vox {
class DriverMMSystem : public DriverPumped {  // sizeof 0x120
public:
    DriverMMSystem();
    virtual ~DriverMMSystem();
    // false when there is no device
    bool Open();
    virtual void Init(void *param);
    virtual void Shutdown();
protected:
    virtual bool Output(const short *samples, int frames);
    virtual int GetBlockFrames() const { return 441; }
private:
    enum { kBlocks = 4 };
    HWAVEOUT m_waveOut;  // 0xD4
    WAVEHDR m_blocks[kBlocks];  // 0xD8
    std::vector<short> m_blockData[kBlocks];
    int m_currentBuffer;  // 0xB8
};
}
#endif

#endif
