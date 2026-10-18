// framework/include/scriptparam.h
#ifndef FRAMEWORK_INCLUDE_SCRIPTPARAM_H
#define FRAMEWORK_INCLUDE_SCRIPTPARAM_H

#include "core/pstring.h"

// An argument handed to a Lua function
struct ScriptParam {  // sizeof 0x20
    enum TypeDef { TINT, TFLOAT, TDOUBLE, TSTRING, TVOID };
    ScriptParam() : type(TVOID), i(0), f(0), d(0) {}
    ScriptParam(int v) : type(TINT), i(v), f(0), d(0) {}
    ScriptParam(float v) : type(TFLOAT), i(0), f(v), d(0) {}
    ScriptParam(double v) : type(TDOUBLE), i(0), f(0), d(v) {}
    ScriptParam(const jet::String &v) : type(TSTRING), i(0), f(0), d(0), s(v) {}
    int type;  // 0x0
    int i;  // 0x4
    float f;  // 0x8
    double d;  // 0x10
    jet::String s;  // 0x18
};

#endif
