// core/jet/include/util/regularexpression.h
#ifndef CORE_JET_INCLUDE_UTIL_REGULAREXPRESSION_H
#define CORE_JET_INCLUDE_UTIL_REGULAREXPRESSION_H

namespace jet {
class RegularExpression {  // sizeof 0x4
public:
    RegularExpression();
    ~RegularExpression();
    bool Compile(const jet::String&);
    bool Compile(const char*);
    bool Match(const jet::String&);
    bool Match(const char*);
private:
    void* data;  // 0x0
};
}

#endif
