// libs/social_framework/src/batch/profilesnsbatch.h
#ifndef LIBS_SOCIAL_FRAMEWORK_SRC_BATCH_PROFILESNSBATCH_H
#define LIBS_SOCIAL_FRAMEWORK_SRC_BATCH_PROFILESNSBATCH_H

namespace social {
class ProfileSNSBatch : public social::Batcher, public social::SSingleton<social::ProfileSNSBatch> {  // sizeof 0x6C
public:
    ProfileSNSBatch();
    virtual ~ProfileSNSBatch();
    virtual bool CanBeBatched(social::Loadable*);
    virtual bool CanBeMerged(const social::BatchSaveRequest&, social::Storable*);
    virtual bool CanBeMerged(const social::BatchLoadRequest&, social::Loadable*);
    virtual bool CanBeSaveBatched(social::Storable*);
protected:
    virtual void PerformRequest(const social::BatchSaveRequest&);
    virtual void PerformRequest(const social::BatchLoadRequest&);
private:
    static bool sOnDataLoaded(void*, bool, sociallib::ClientSNSEnum);
};
}

#endif
