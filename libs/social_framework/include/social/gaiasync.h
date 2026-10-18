// libs/social_framework/include/social/gaiasync.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_GAIASYNC_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_GAIASYNC_H

class GaiaSync {  // sizeof 0x1
public:
    GaiaSync();
    static void Init();
    static void DeInit();
    static void Update();
    static void PrepareCallback(void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*&, void*&);
    static void RunCallback(int);
    static void UnregisterCaller(void*);
private:
    static void OnGaiaCallback(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static std::vector<GaiaSync::SCallback,std::allocator<GaiaSync::SCallback> > s_callbacks;
    static std::queue<GaiaSync::SReadyCallback,std::deque<GaiaSync::SReadyCallback,std::allocator<GaiaSync::SReadyCallback> > > s_readyCallbacks;
    static int s_pendingCallbacks;
};

#endif
