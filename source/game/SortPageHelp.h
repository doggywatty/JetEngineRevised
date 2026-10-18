// source/game/SortPageHelp.h
#ifndef SOURCE_GAME_SORTPAGEHELP_H
#define SOURCE_GAME_SORTPAGEHELP_H

// What the pages of the end screen are sorted by (the .so's SortPageHelp, 0x10 bytes): marks that a mission was done (0), that
// a location was unlocked (1), and (3) that 900 seconds went by since the last time (Game::OnUpdate); initPageNum() of the
// end screen (Menu_ResultEndScreen::OnEnter) reads the marks and clears them. Not ported: that reader and the timer.
class SortPageHelp {
public:
    SortPageHelp() : m_missionDone(false), m_locationUnlocked(false), m_flag2(false), m_timer(false) {}  // @0x497ea0
    static SortPageHelp *GetInstance() {                                                                 // @0x497ecc
        static SortPageHelp *s_instance = 0;
        if (!s_instance) s_instance = new SortPageHelp;
        return s_instance;
    }
    bool m_missionDone, m_locationUnlocked, m_flag2, m_timer;  // 0x0, 0x1, 0x2, 0x3
};

#endif
