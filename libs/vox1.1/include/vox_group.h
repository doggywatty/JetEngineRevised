// libs/vox1.1/include/vox_group.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_GROUP_H
#define LIBS_VOX1_1_INCLUDE_VOX_GROUP_H

namespace vox {
class GroupManager {  // sizeof 0x14
public:
    GroupManager();
    ~GroupManager();
    unsigned AddGroup(vox::group::CreationSettings&);
    bool ReconfigureGroup(unsigned, vox::group::CreationSettings&);
    unsigned GetGroupId(const char*);
    bool GetGroupName(unsigned, char*, int);
    bool IsChild(unsigned, unsigned);
    bool IsGroupValid(unsigned);
    bool GetEnable(unsigned);
    bool SetEnable(unsigned, bool, float);
    float GetVolume(unsigned);
    bool SetVolume(unsigned, float, float);
    float GetPitch(unsigned);
    bool SetPitch(unsigned, float, float);
    void Update(float);
    float GetEffectiveVolume(unsigned);
    float GetEffectivePitch(unsigned);
private:
    std::vector<vox::Group,vox::SAllocator<vox::Group,0> > m_groups;  // 0x0
};
}

#endif
