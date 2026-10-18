// libs/social_framework/src/batch/gameprofilebatch.h
#ifndef LIBS_SOCIAL_FRAMEWORK_SRC_BATCH_GAMEPROFILEBATCH_H
#define LIBS_SOCIAL_FRAMEWORK_SRC_BATCH_GAMEPROFILEBATCH_H

namespace social {
class GameProfileBatch : public social::Batcher, public social::SSingleton<social::GameProfileBatch> {  // sizeof 0x80
public:
    GameProfileBatch();
    virtual ~GameProfileBatch();
    virtual bool CanBeBatched(social::Loadable*);
    virtual bool CanBeMerged(const social::BatchSaveRequest&, social::Storable*);
    virtual bool CanBeMerged(const social::BatchLoadRequest&, social::Loadable*);
    virtual bool CanBeSaveBatched(social::Storable*);
protected:
    virtual void PerformRequest(const social::BatchSaveRequest&);
    virtual void PerformRequest(const social::BatchLoadRequest&);
private:
    static void sOnProfileLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnProfileSaved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnVisibilitySaved(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    void FinishSave(bool);
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_profileResponses;  // 0x6C
};
}

#endif
