// core/jet/src/video/colorcorrection.cpp
#include "video/colorcorrection.h"

bool g_ColorCorrectionEnable = false;
bool g_forceUseColorCorrection = false;
// @0x3bad58: the static initialiser sets every matrix to the identity (math::mat4's constructor)
math::mat4<float> g_ColorCorrectionMatrix[8];
int g_ActiveColorProfile = 1, g_LastActiveColorProfile = 1;
float g_TransitionTime = -1.0f;
