// source/game/gamestates/GS_Loading.h
#ifndef SOURCE_GAME_GAMESTATES_GS_LOADING_H
#define SOURCE_GAME_GAMESTATES_GS_LOADING_H

#include <map>
#include <vector>
#include "GameState.h"
#include "game/interfaces/interfaceobjects/BasicPageOwner.h"

namespace jet::stream { class IStream; }

// The loading screen and the steps behind it: the first launch shows the logos, then the loading tips while the
// level and its libraries are loaded a few milliseconds a frame; then the state it goes to is pushed.
// The bonus tip of the first loading (it needs the Bonus entity) and the online / DLC steps are not ported yet.
class GS_Loading : public GameState, public BasicPageOwner {  // sizeof 0xB0
public:
    enum GoToEnum { GoTo_MainMenu = 0, GoTo_MainMenuFromGame = 1, GoTo_Gameplay = 2, GoTo_Movie = 3, GoTo_Options = 5 };
    enum LoadingStepsEnum { Steps_Boot = 0, Steps_Menu = 1, Steps_Gameplay = 2, Steps_Movie = 3, Steps_Reload = 4 };
    // one screen of the first launch after the other: the two logos, the splash and the tips
    enum EScreensStepsDef { Screen_Universal = 0, Screen_Illumination = 1, Screen_Splash = 2, Screen_Loading = 3 };
    GS_Loading(GoToEnum, LoadingStepsEnum, std::map<jet::String, jet::stream::IStream*> &libs, jet::stream::IStream *level);
    GS_Loading(GoToEnum, LoadingStepsEnum, const std::vector<jet::String> &libs, const std::vector<jet::String> &levels);
    virtual ~GS_Loading();
    virtual const jet::String &GetStateName() { return k_stateName; }
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    static const jet::String k_stateName;
    static bool s_showingOpenCredits;
private:
    void InitLoading(GoToEnum, LoadingStepsEnum);
    void InitScreens();
    void LoadScreen(EScreensStepsDef);
    void LoadScreen_EnableSplashLogo();
    void UpdateScreens();
    void UpdateLoadingTips();
    // creates the streams of the files not opened yet; true when all are
    bool LoadLevelStreams();
    // one step of the list; false while it needs more frames
    bool _UpdateLoadingStep();
    void Finish();
    const int *m_loadingSteps;  // 0x14
    int m_loadingStepsLeft;  // 0x18
    long long m_startTime;  // 0x20
    long long m_loadingTime;  // 0x28
    int m_stepIndex;  // 0x38
    unsigned m_loadingSubStep;  // 0x3C
    GoToEnum m_goTo;  // 0x40
    int m_totalTips;  // 0x44
    std::map<jet::String, jet::stream::IStream*> m_libStreams;  // 0x48
    std::map<jet::String, jet::stream::IStream*> m_levelStreams;  // 0x60
    bool m_streamsAreOwned;  // 0x78
    bool m_waitingSkip;  // 0x79
    bool m_skipPressed;  // 0x7A
    bool m_doneLoading;  // 0x7B
    int m_loadingUpdateTimeLeft;  // 0x7C
    int m_postponeLoading;  // 0x80
    EScreensStepsDef m_currentScreen;  // 0x84
    long long m_screenTimer;  // 0x88
    bool m_screenChanged;  // 0x90
    int m_totalDownloadMessages;  // 0x94
    bool m_tipDirty;  // 0x99
    bool m_noInternetTip;  // 0xA9
};

#endif
