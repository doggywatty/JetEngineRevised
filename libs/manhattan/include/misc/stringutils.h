// libs/manhattan/include/misc/stringutils.h
#ifndef LIBS_MANHATTAN_INCLUDE_MISC_STRINGUTILS_H
#define LIBS_MANHATTAN_INCLUDE_MISC_STRINGUTILS_H

namespace manhattan::misc {
class StringUtils {  // sizeof 0x1
public:
    static bool StringIsNumeric(const char*, int);
private:
    StringUtils(const manhattan::misc::StringUtils&);
    StringUtils();
    manhattan::misc::StringUtils &operator=(const manhattan::misc::StringUtils&);
};
}

#endif
