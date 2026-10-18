// source/game/online/pushnotifications/PushNotificationsMgr.h
#ifndef SOURCE_GAME_ONLINE_PUSHNOTIFICATIONS_PUSHNOTIFICATIONSMGR_H
#define SOURCE_GAME_ONLINE_PUSHNOTIFICATIONS_PUSHNOTIFICATIONSMGR_H

namespace online {
class PushNotificationsMgr : public Singleton<online::PushNotificationsMgr> {  // sizeof 0x4
public:
    bool DeviceSupportsLocalNotifications();
    bool WasAskedForRegistration();
    void RegisterForPushNotifications();
    bool HasUserEnabledNotificationsInDeviceSettings();
    bool WereNotificationsInDeviceSettingsEnabledLastTime();
    void SetWereNotificationsInDeviceSettingsEnabledLastTime(bool);
    void ScheduleLocalNotification(const char*, const char*, const char*, const char*, int, online::PushNotificationsMgr::PushNot_RepeatIntervals);
    void CancelAllLocalNotifications();
    void SetApplicationIconBadgeNumber(int);
private:
    int GetRepeatInterval(online::PushNotificationsMgr::PushNot_RepeatIntervals);
};
}

#endif
