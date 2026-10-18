// core/jet/include/text/fontgenerator.h
#ifndef CORE_JET_INCLUDE_TEXT_FONTGENERATOR_H
#define CORE_JET_INCLUDE_TEXT_FONTGENERATOR_H

namespace jet::text {
class FontGenerator {  // sizeof 0x1
public:
    FontGenerator();
    void Update(const jet::text::PageGenerator*, jet::text::Font*);
};
}

#endif
