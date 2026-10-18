// source/game/gamestates/GS_LoadRemoteLevel.h
#ifndef SOURCE_GAME_GAMESTATES_GS_LOADREMOTELEVEL_H
#define SOURCE_GAME_GAMESTATES_GS_LOADREMOTELEVEL_H

class GS_LoadRemoteLevel : public GameState {  // sizeof 0x2C
public:
    virtual const jet::String &GetStateName();
    GS_LoadRemoteLevel();
    virtual ~GS_LoadRemoteLevel();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    static const jet::String k_stateName;
private:
    GS_LoadRemoteLevel::eState m_state;  // 0xC
    jet::String m_serverAddress;  // 0x10
    const Touch *m_touch;  // 0x14
    std::vector<GS_LoadRemoteLevel::LevelOption,std::allocator<GS_LoadRemoteLevel::LevelOption> > m_options;  // 0x18
    static bool s_syncCache;
    static bool s_syncRemoteResources;
};

#endif
