// source/game/colorprofiles.h
#ifndef SOURCE_GAME_COLORPROFILES_H
#define SOURCE_GAME_COLORPROFILES_H

#include "video/colorcorrection.h"

// g_ColorCorrectionEnable, g_ColorCorrectionMatrix, g_ActiveColorProfile, g_LastActiveColorProfile and
// g_TransitionTime are declared there: the driver reads them too.

// The kinds of a device's colour profile, in the order of g_ColorCorrectionMatrix: colorProfilePostfix @0xcff024
extern const char *colorProfilePostfix[8];

// @0x3e775c: replaces every `from` in the string, returns how many were
int ReplaceChar(char *s, char from, char to);
// @0x3e7938: the grading matrix of a profile: a hue rotation about the grey axis, the gain per channel and the
// saturation per channel, with the offset in the translation
math::mat4<float> ComputeColorOffsetMatrix(const math::vec3<float> &a, const math::vec3<float> &b, const math::vec3<float> &c,
                                           const math::vec3<float> &d, float e);
// @0x3e7eac: the matrix `index` from the last line of a profile file
void LoadProfile(const char *file, int index);
// @0x3e83e8: the eight matrices of the files <base><colorProfilePostfix[i]>
void LoadProfile(const char *base);
// @0x3e8470: the profile files of the running device, and whether the colour correction is on
void LoadProfile();

#endif
