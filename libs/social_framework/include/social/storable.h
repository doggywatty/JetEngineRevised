// libs/social_framework/include/social/storable.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_STORABLE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_STORABLE_H

namespace social {
class Storable : public social::Loadable {  // sizeof 0xBC
public:
    Storable(const social::User*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::ReloadStrategy*);
    virtual ~Storable();
    bool IsUpdated();
    void Save();
    const social::OnlineStatus<enum social::Storable::SaveState> &GetSaveStatus();
protected:
    void SetChanged();
    void OnSaved(bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual void SaveImpl() = 0;
    virtual void LoadImpl();
    virtual void SaveableLoadImpl() = 0;
    virtual void UnLoadImpl();
    virtual void SaveableUnLoadImpl() = 0;
    virtual void CancelLoadImpl();
    virtual void SaveableCancelLoadImpl() = 0;
private:
    static bool sOnLoaded(void*, bool, social::OnlineEventData);
    static bool sOnSaved(void*, bool, social::OnlineEventData);
protected:
    social::OnlineStatus<enum social::Storable::SaveState> m_saveStatus;  // 0x8C
private:
    social::Storable::SaveState m_nextState;  // 0xB8
};
}

#endif
