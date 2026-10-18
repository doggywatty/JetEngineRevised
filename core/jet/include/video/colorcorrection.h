// core/jet/include/video/colorcorrection.h
#ifndef CORE_JET_INCLUDE_VIDEO_COLORCORRECTION_H
#define CORE_JET_INCLUDE_VIDEO_COLORCORRECTION_H

#include "smath.h"

// The colour grading shared by the game and the GLES2 driver. The game fills the matrices from the
// device's colour profiles (LoadProfile @0x3e8470) and chooses the profile of the location; the driver
// uploads them as the auto uniforms mesh.colorCorrection_Minion / _Background / _Banana.
// They are the original's globals of the same names, defined here so that the engine does not depend on
// the game: g_ColorCorrectionEnable @0xd0a204, g_forceUseColorCorrection @0xd0a205,
// g_ColorCorrectionMatrix @0xd0a208, g_ActiveColorProfile @0xcff004, g_LastActiveColorProfile @0xcff008,
// g_TransitionTime @0xcff00c.
extern bool g_ColorCorrectionEnable;
extern bool g_forceUseColorCorrection;
// one matrix per kind of profile: minion, onlap, res, macho, beach, banana, resNight, mall
extern math::mat4<float> g_ColorCorrectionMatrix[8];
// the profile (index of g_ColorCorrectionMatrix) in use and the one before it; switching takes g_TransitionTime
extern int g_ActiveColorProfile, g_LastActiveColorProfile;
extern float g_TransitionTime;

#endif
