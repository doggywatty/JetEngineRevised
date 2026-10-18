// libs/manhattan/include/dlc/manhattaninstallerperformancesettings.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_MANHATTANINSTALLERPERFORMANCESETTINGS_H
#define LIBS_MANHATTAN_INCLUDE_DLC_MANHATTANINSTALLERPERFORMANCESETTINGS_H

namespace manhattan::dlc {
class ManhattanInstallerPerformanceSettings {  // sizeof 0xC
public:
    ManhattanInstallerPerformanceSettings(const manhattan::dlc::ManhattanInstallerPerformanceSettings&);
    ManhattanInstallerPerformanceSettings(manhattan::dlc::ManhattanInstallerPerformanceSettings::ExecutionMode);
    virtual ~ManhattanInstallerPerformanceSettings();
    unsigned GetDecoderBufferSize(manhattan::dlc::ManhattanInstallerPerformanceSettings::BufferType);
    glwebtools::Thread::Priority GetThreadPriority(manhattan::dlc::ManhattanInstallerPerformanceSettings::TaskType);
    unsigned GetDecoderSleepPeriod();
    void SetExecutionMode(manhattan::dlc::ManhattanInstallerPerformanceSettings::ExecutionMode);
private:
    manhattan::dlc::ManhattanInstallerPerformanceSettings::ExecutionMode m_modeInternal;  // 0x4
    manhattan::dlc::ManhattanInstallerPerformanceSettings::ExecutionMode &m_mode;  // 0x8
};
}

#endif
