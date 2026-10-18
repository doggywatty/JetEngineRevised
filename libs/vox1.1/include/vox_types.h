// libs/vox1.1/include/vox_types.h
// The small value types and enums every part of the vox engine shares. The .so is stripped of the vox symbols, so the
// numeric values are the ones the soundpack loader (vox_soundpack_xml.cpp @0x5b5b00..0x5bb800) and the engine store.
#ifndef LIBS_VOX1_1_INCLUDE_VOX_TYPES_H
#define LIBS_VOX1_1_INCLUDE_VOX_TYPES_H

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace vox {

enum VoxMemHint { kVoxMemHintDefault = 0 };

// "loadingflags" of a sound of the soundpack @0x5b7e14: none / "load to ram" / "load and decode"
enum VoxSourceLoadingFlags { kLoadNone = 0, kLoadToRam = 1, kLoadAndDecode = 2 };

// "mode3d" of a sound or a group @0x5b6e10: 'y' is a 3D sound, 'r' is a 3D sound relative to the listener
enum Vox3DSoundType { k3DOff = 0, k3DWorld = 1, k3DRelative = 2 };

// "format" of a sound @0x5b7d18: pcm/adpcm 1, ogg 2, mpc/mpc8 3, vxn 4, bcwav 5. It is also the index of the decoder type
// that VoxEngine::Initialize registers (0 raw, 1 mswav, 2 stb_vorbis, 3 mpc8, 4 native "VoxN"); the stream type of every
// sound is 0 (a file), 1 is a memory buffer, 2 a segment of a native file
enum VoxSoundFormat { kFormatNone = -1, kFormatRaw = 0, kFormatWav = 1, kFormatOgg = 2, kFormatMpc = 3, kFormatVxn = 4, kFormatBcwav = 5 };
enum VoxStreamType { kStreamCFile = 0, kStreamMemoryBuffer = 1, kStreamNativeSegment = 2 };

enum VoxOutputMode { kOutputDefault = 0, kOutputMono = 1, kOutputStereo = 2, kOutputSurround = 3 };

// the state of an emitter that the user is told about (RegisterForEmitterStateChangeNotification)
enum EmitterExternState { kExternStateStopped = 0, kExternStatePlaying = 1, kExternStatePaused = 2, kExternStateDone = 3, kExternStateDead = 4 };

// the internal state of an emitter (EmitterObj::GetState)
enum EmitterState { kEmitterInit = 0, kEmitterStopped = 1, kEmitterPlaying = 2, kEmitterPaused = 3, kEmitterDone = 4 };

// what the driver says of a source (DriverSourceInterface::GetState)
enum DriverSourceState { kSourceStopped = 0, kSourcePlaying = 1, kSourcePaused = 2 };

// 3D parameters of an emitter (Set3DEmitterParameteri / f)
enum Vox3DEmitterParameterId {
    k3DEmitterRelativeToListener = 0, k3DEmitterMaxDistance = 1, k3DEmitterReferenceDistance = 2, k3DEmitterRolloffFactor = 3,
    k3DEmitterInnerConeAngle = 4, k3DEmitterOuterConeAngle = 5, k3DEmitterOuterConeGain = 6, k3DEmitterPosition = 7,
    k3DEmitterVelocity = 8, k3DEmitterDirection = 9
};

// 3D general parameters (Set3DGeneralParameter); SoundMgr::Initialize @0x63bcec sets 2 (int) and 0 / 1 (float)
enum Vox3DGeneralParameterId {
    k3DGeneralDopplerFactor = 0, k3DGeneralSpeedOfSound = 1, k3DGeneralDistanceModel = 2, k3DGeneralEnhanced3d = 3
};

struct VoxVector3f {
    float x, y, z;
    VoxVector3f() : x(0), y(0), z(0) {}
    VoxVector3f(float a, float b, float c) : x(a), y(b), z(c) {}
};

// What a decoder tells of its track (DecoderCursorInterface +4..+0x10 @0x5e24fc)
struct TrackParams {
    int numChannels;
    int sampleRate;
    int bitsPerSample;
    int numSamples;
    TrackParams() : numChannels(0), sampleRate(0), bitsPerSample(0), numSamples(0) {}
};

// the user data an emitter / a data source carries
struct EmitterHandleUserData { void *data; EmitterHandleUserData() : data(0) {} };
struct DataHandleUserData { void *data; DataHandleUserData() : data(0) {} };

}  // namespace vox

#endif
