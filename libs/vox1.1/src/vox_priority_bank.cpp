// libs/vox1.1/src/vox_priority_bank.cpp
#include <cctype>
#include "vox_internal.h"

namespace vox {

unsigned PriorityBankManager::AddPriorityBank(const priority_bank::CreationSettings &s) {
    Bank b;
    b.name = s.m_name ? s.m_name : "";
    b.settings = s;
    b.settings.m_name = 0;
    m_banks.push_back(b);
    return (unsigned)m_banks.size() - 1;
}

bool PriorityBankManager::SetPriorityBank(unsigned id, const priority_bank::CreationSettings &s) {
    if (id >= m_banks.size()) return false;
    Bank &b = m_banks[id];
    if (s.m_name) b.name = s.m_name;
    b.settings = s;
    b.settings.m_name = 0;
    return true;
}

unsigned PriorityBankManager::GetPriorityBankId(const char *name) const {
    if (!name) return (unsigned)-1;
    for (size_t i = 0; i < m_banks.size(); i++) {
        const std::string &n = m_banks[i].name;
        size_t k = 0;
        for (; k < n.size() && name[k]; k++)
            if (tolower((unsigned char)n[k]) != tolower((unsigned char)name[k])) break;
        if (k == n.size() && !name[k]) return (unsigned)i;
    }
    return (unsigned)-1;
}

// An emitter that starts in a bank that already has its maxPlayback sounds either takes the place of one of them, as the
// behaviour of the bank says (the sound that is stolen is stopped), or is not played.
bool PriorityBankManager::AddEmitter(unsigned id, EmitterObj *e) {
    if (id >= m_banks.size()) return true;
    Bank &b = m_banks[id];
    if (b.settings.m_maxPlayback > 0 && b.elements.size() >= b.settings.m_maxPlayback) {
        std::list<EmitterObj*>::iterator victim = b.elements.end();
        switch (b.settings.m_behaviour) {
            case priority_bank::kStealOldest:
                victim = b.elements.begin();
                break;
            case priority_bank::kStealLowestPriority:
            case priority_bank::kStealLowPriorityOrOldestSamePriority: {
                // the lowest priority; the oldest of them for the second behaviour (the list is ordered by age), the newest
                // for the first one, which keeps the sounds that were there first when they are as important
                for (std::list<EmitterObj*>::iterator it = b.elements.begin(); it != b.elements.end(); ++it) {
                    if (victim == b.elements.end()) victim = it;
                    else if ((*it)->GetPriority() < (*victim)->GetPriority()) victim = it;
                    else if ((*it)->GetPriority() == (*victim)->GetPriority() && b.settings.m_behaviour == priority_bank::kStealLowestPriority) victim = it;
                }
                if (victim != b.elements.end() && (*victim)->GetPriority() > e->GetPriority()) return false;
                break;
            }
            case priority_bank::kStealQuietest:
                for (std::list<EmitterObj*>::iterator it = b.elements.begin(); it != b.elements.end(); ++it)
                    if (victim == b.elements.end() || (*it)->GetAttenuation() < (*victim)->GetAttenuation()) victim = it;
                break;
            default:
                return false;
        }
        if (victim == b.elements.end()) return false;
        EmitterObj *stolen = *victim;
        b.elements.erase(victim);
        stolen->SetInPriorityBank(false);
        stolen->Kill();
    }
    b.elements.push_back(e);
    e->SetInPriorityBank(true);
    return true;
}

bool PriorityBankManager::RemoveEmitter(unsigned id, EmitterObj *e) {
    if (id >= m_banks.size()) return false;
    m_banks[id].elements.remove(e);
    e->SetInPriorityBank(false);
    return true;
}

}  // namespace vox
