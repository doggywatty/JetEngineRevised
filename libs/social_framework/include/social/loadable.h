// libs/social_framework/include/social/loadable.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_LOADABLE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_LOADABLE_H

namespace social {
class Loadable : public social::SimpleEventDispatcher<social::OnlineEventData> {  // sizeof 0x8C
    Loadable(social::Loadable&);
public:
    Loadable(const social::User*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, social::ReloadStrategy*);
    virtual ~Loadable();
    void AddDependency(social::Loadable*);
    bool IsLoaded();
    void Load();
    void Unload();
    void CancelLoad();
    void RegisterLoadCallback(bool (void*, bool, social::OnlineEventData)*, void*);
    const social::OnlineStatus<enum social::Loadable::LoadState> &GetStatus();
    bool NeedsReload();
    const social::User *GetOwner();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetKey();
    virtual social::Batcher *GetBatch();
protected:
    virtual void LoadImpl() = 0;
    virtual void UnLoadImpl() = 0;
    virtual void CancelLoadImpl() = 0;
    social::ReloadStrategy *GetReloadStrategy();
private:
    social::Loadable &operator=(const social::Loadable&);
    static bool sOnDependencyLoaded(void*, bool, social::OnlineEventData);
    static bool sOnUnloaded(void*, bool, social::OnlineEventData);
protected:
    social::OnlineStatus<enum social::Loadable::LoadState> m_status;  // 0x20
private:
    std::vector<social::Loadable *,std::allocator<social::Loadable *> > m_dependencies;  // 0x4C
    int m_pendingDependencies;  // 0x60
    const social::User *m_owner;  // 0x64
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0x68
    social::ReloadStrategy *m_reloadStrategy;  // 0x88
};
}

#endif
