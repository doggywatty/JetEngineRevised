// source/game/gamemanagers/IGTitleUIMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_IGTITLEUIMGR_H
#define SOURCE_GAME_GAMEMANAGERS_IGTITLEUIMGR_H

class IGTitleUIMgr : public Singleton<IGTitleUIMgr> {  // sizeof 0x28
public:
    IGTitleUIMgr();
    virtual ~IGTitleUIMgr();
    void Update(int);
    void Render(const jet::scene::Camera&, jet::video::Painter&);
    void TrySettingLocation(const jet::String&);
    void TrySettingDistanceInfo(int);
    void PushTitle(const jet::String&);
private:
    IGTitleUIBox *m_titleUIBox;  // 0x4
    std::vector<IGTitleUIMgr::STitleInfo,std::allocator<IGTitleUIMgr::STitleInfo> > m_UITitleInfo;  // 0x8
    jet::String m_locationName;  // 0x1C
    static const int k_milestoneValue;
    int m_lastMilestone;  // 0x20
    bool m_showNewDistanceRecord;  // 0x24
};

#endif
