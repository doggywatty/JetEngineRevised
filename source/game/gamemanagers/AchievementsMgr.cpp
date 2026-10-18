// source/game/gamemanagers/AchievementsMgr.cpp
//
// Not ported: CheatsMgr's list of the achievements (ResetAchievementsEntries / AddAchievementEntry).
#include "AchievementsMgr.h"
#include "game/Game.h"
#include "game/entities/Achievement.h"
#include "game/entities/AchievementSet.h"
#include "project.h"

const jet::String AchievementsMgr::k_saveRecordName("AchievementsMgr");

typedef AchievementsMgrSaveData::SAchievementInfo Info;

// @0x540514: the achievements the save has done are marked so; the others start to count
void AchievementsMgr::Reset() {
    AchievementSet *set = AchievementSet::GetInstance();
    if (!set) return;
    std::vector<Achievement*> &list = AchievementSet::GetInstance()->GetAchievements();
    for (size_t i = 0; i < list.size(); i++) {
        Achievement *a = list[i];
        bool done = false;
        for (size_t j = 0; j < m_data.achievements.size(); j++)
            if (a->GetName().Equals(m_data.achievements[j].id)) { done = true; break; }
        if (done) a->MarkAsPreviouslyCompleted();
        else a->OnStarted();
    }
}

// @0x5405ec
void AchievementsMgr::ResetData(AchievementsMgrSaveData &d) const { d.achievements.clear(); }

// @0x540634
void AchievementsMgr::SaveDataInStream(jet::stream::IStreamW &s, AchievementsMgrSaveData &d) const {
    int version = k_saveGameVersion;
    s << version;
    SerializeV2(s, d);
}

// @0x540680
bool AchievementsMgr::LoadDataFromStream(jet::stream::IStream &s, AchievementsMgrSaveData &d) const {
    int version = 0;
    s >> version;
    if (version == 1) return DeserializeV1(s, d);
    if (version == 2) return DeserializeV2(s, d);
    return false;
}

// @0x5406ec
void AchievementsMgr::SerializeV1(jet::stream::IStreamW &s, AchievementsMgrSaveData &d) const {
    s << (int)k_signatureV1;
    unsigned n = (unsigned)d.achievements.size();
    s << n;
    for (size_t i = 0; i < d.achievements.size(); i++) {
        jet::WriteString(s, d.achievements[i].id);
        s << d.achievements[i].prizeClaimed;
    }
}

// @0x540778
bool AchievementsMgr::DeserializeV1(jet::stream::IStream &s, AchievementsMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV1) return false;
    int n = 0;
    s >> n;
    d.achievements.assign(n, Info());
    for (int i = 0; i < n; i++) {
        Info info;
        s >> info.id;
        s >> info.prizeClaimed;
        d.achievements[i] = info;
    }
    return true;
}

// @0x540854
void AchievementsMgr::SerializeV2(jet::stream::IStreamW &s, AchievementsMgrSaveData &d) const {
    s << (int)k_signatureV2;
    unsigned n = (unsigned)d.achievements.size();
    s << n;
    for (size_t i = 0; i < d.achievements.size(); i++) {
        jet::WriteString(s, d.achievements[i].id);
        s << d.achievements[i].prizeClaimed;
        s << d.achievements[i].igNotifyShown;
    }
}

// @0x5408ec
bool AchievementsMgr::DeserializeV2(jet::stream::IStream &s, AchievementsMgrSaveData &d) const {
    int sig = 0;
    s >> sig;
    if (sig != k_signatureV2) return false;
    int n = 0;
    s >> n;
    d.achievements.assign(n, Info());
    for (int i = 0; i < n; i++) {
        Info info;
        s >> info.id;
        s >> info.prizeClaimed;
        s >> info.igNotifyShown;
        d.achievements[i] = info;
    }
    return true;
}

// @0x5409cc: a new achievement done is noted (its prize not taken, its notification not shown)
void AchievementsMgr::OnAchievementJustCompleted(const jet::String &name) {
    for (size_t i = 0; i < m_data.achievements.size(); i++)
        if (m_data.achievements[i].id.Equals(name)) return;
    Info info;
    info.id = name;
    info.prizeClaimed = false;
    info.igNotifyShown = false;
    m_data.achievements.push_back(info);
    Game::GetInstance()->Save(false, false);
}

// @0x540a80: the notification of the achievements done whose prize and notification are pending
void AchievementsMgr::OnStartGameShowPendingIGNotifies() {
    for (size_t i = 0; i < m_data.achievements.size(); i++) {
        const Info &info = m_data.achievements[i];
        if (info.prizeClaimed || info.igNotifyShown) continue;
        if (clara::DataEntity *e = clara::Project::GetInstance()->FindEntityByName(info.id))
            static_cast<Achievement*>(e)->ShowIngameNotification();
    }
}

// @0x540af4
void AchievementsMgr::MarkIGNotifyShownFor(const jet::String &name) {
    for (size_t i = 0; i < m_data.achievements.size(); i++) {
        Info &info = m_data.achievements[i];
        if (!info.igNotifyShown && info.id.Equals(name)) {
            info.igNotifyShown = true;
            Game::GetInstance()->Save(true, true);
            return;
        }
    }
}

// @0x540b7c: an achievement done whose prize is there to take
bool AchievementsMgr::HasAnyPrizeToClaim() {
    for (size_t i = 0; i < m_data.achievements.size(); i++)
        if (!m_data.achievements[i].prizeClaimed) return true;
    return false;
}

// @0x540bc0
bool AchievementsMgr::WasPrizeClaimedFor(const jet::String &name) {
    for (size_t i = 0; i < m_data.achievements.size(); i++)
        if (m_data.achievements[i].id.Equals(name)) return m_data.achievements[i].prizeClaimed;
    return false;
}

// @0x540c14: the achievement not yet noted is (done, prize claimed, notification shown)
void AchievementsMgr::SetPrizeClaimedFor(const jet::String &name) {
    for (size_t i = 0; i < m_data.achievements.size(); i++)
        if (m_data.achievements[i].id.Equals(name)) {
            m_data.achievements[i].prizeClaimed = true;
            Game::GetInstance()->Save(false, false);
            return;
        }
    Info info;
    info.id = name;
    info.prizeClaimed = true;
    info.igNotifyShown = true;
    m_data.achievements.push_back(info);
    Game::GetInstance()->Save(false, false);
}

// @0x540cf4
unsigned AchievementsMgr::GetCountCompletedAchievements() { return (unsigned)m_data.achievements.size(); }
