// libs/social_framework/include/social/alerts/alertmanager.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_ALERTS_ALERTMANAGER_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_ALERTS_ALERTMANAGER_H

namespace social {
class AlertManager : public social::SSingleton<social::AlertManager>, public social::SimpleEventDispatcher<social::Alert *> {  // sizeof 0x38
public:
    AlertManager();
    virtual ~AlertManager();
    const std::vector<social::Alert,std::allocator<social::Alert> > &GetAlertList();
    void CleanAlerts();
    void AddConnectionAlert();
    void AddMessageAlert(social::TransportType);
    void StartListening();
    void StopListening();
private:
    void AddAlert(social::Alert&);
    static void sOnGetAlerts(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void KairosCallback(gaia::State, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*);
    std::vector<social::Alert,std::allocator<social::Alert> > m_alertList;  // 0x24
};
}

#endif
