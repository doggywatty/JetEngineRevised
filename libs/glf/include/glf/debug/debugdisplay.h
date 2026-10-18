// libs/glf/include/glf/debug/debugdisplay.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUG_DEBUGDISPLAY_H
#define LIBS_GLF_INCLUDE_GLF_DEBUG_DEBUGDISPLAY_H

namespace glf {
struct DebugDisplay {  // sizeof 0x24
    static glf::DebugDisplay &Get();
    DebugDisplay();
    ~DebugDisplay();
    void reset();
    void flush();
    int getLineHeight();
    void setLineHeight(int);
    int getCharSpacing();
    void setCharSpacing(int);
    void setColor(unsigned);
    unsigned getColor();
    void drawCharacter(char, int, int, unsigned);
    void drawString(const char*, int, int, unsigned);
    void fillRect(int, int, int, int);
    void fillCircle(int, int, int);
    void drawLines(std::vector<glf::Point<short>,std::allocator<glf::Point<short> > >&);
    glf::DebugDisplayAdapter *setAdapter(glf::DebugDisplayAdapter*);
    glf::DebugDisplayAdapter *getAdapter();
protected:
    void drawGlyph(int, int, int, unsigned);
private:
    void drawGlyphs(const char*, int, int);
    void drawString2(const char*, int, int);
    static glf::DebugDisplay *sDebugDisplay;
    std::vector<glf::DrawInfo,std::allocator<glf::DrawInfo> > mDrawInfos;  // 0x0
    glf::DebugDisplayAdapter *mAdapter;  // 0x14
    unsigned mColor;  // 0x18
    int mLineHeight;  // 0x1C
    int mCharSpacing;  // 0x20
};
}

#endif
