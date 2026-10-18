// source/game/leveltemplate/LevelTemplateMgr.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATEMGR_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATEMGR_H

#include <list>
#include <map>
#include <vector>
#include "core/pstring.h"
#include "smath.h"
#include "game/gameenums.h"
#include "game/leveltemplate/leveltemplatesequenceprovider.h"

class LevelTemplateDef;
class LevelTemplateInstance;
class GameplayTemplateInstance;

// A template is shown while its end is ahead of the minion and its start is within the fog (true when that changed)
bool UpdateTemplateVisibility(LevelTemplateInstance*, unsigned distance, float end, float prevEnd);
namespace clara { class Folder; class DataEntity; }

// Builds the run: the level templates the sequence names, one after the other, and the backgrounds around them. The templates and the
// backgrounds come from the libraries of the project, by name ("001_LAB" is template "001", "Background_LaboStart" a background).
class LevelTemplateMgr {
public:
    // a template that was made: its definition and the location it was made in
    struct HistoryData {
        LevelTemplateDef *def;
        jet::String location;
    };
    // where the next background starts: the end of the last template, how it turns, and the sizes along the run
    struct BackgroundInfo {
        math::vec3<float> position;
        float angle;
        int bifurcation;
        float size, size2;
    };
    // what the sequence says of a template to come, ready in the cache before it is made
    struct LevelTemplateData {
        int templateId;                // 0x0
        GameEntity *camera;            // 0x4
        int cameraTransitionTime;      // 0x8
        const SpeedDef *speed;         // 0xC
        jet::String location;          // 0x10
        Bifurcation bifurcation;       // 0x14
        bool pause;                    // 0x18
        jet::String nodeLabel, nodeId; // 0x1C, 0x20
        int nodeGraphId;               // 0x24
        explicit LevelTemplateData(LevelTemplateSequenceProvider *p) { SetData(p); }
        void SetData(LevelTemplateSequenceProvider*);
    };
    explicit LevelTemplateMgr(LevelTemplateSequenceProvider *p);
    ~LevelTemplateMgr();
    // reads the libraries of the project into the definitions (again when forced)
    static void BuildLevelTemplates(bool force = false);
    static void SetLevelTemplatesData();
    static void CleanLevelTemplates();
    // drops the definitions of those libraries
    static void CleanLevelTemplates(const std::vector<jet::String> &libraries);
    static Bifurcation GetBifurcation(int templateId);
    static bool GetPause(int templateId);
    static LevelTemplateDef *GetLevelTemplateDef(int templateId);
    // the location a background library is in (-1 when there is none)
    static int GetBGLocation(const jet::String &name);
    static const std::map<jet::String, LevelTemplateDef*> &GetBackgroundTemplates() { return s_backgroundTemplates; }
    const std::vector<HistoryData> &GetHistory() const { return m_history; }
    void CacheInitialLevelSequence();
    void PopulateCacheSequenceBuffer();
    void ResumeCreation();
    // the templates that precede the distance are taken as made; what they cover is returned
    unsigned AdvanceLevelSequence(unsigned distance);
    void InitLevelParts();
    void CleanLevelParts();
    GameplayTemplateInstance *GetCurrentLevelTemplateInstance() const { return m_current; }
    // the templates are removed sooner while the level runs fast (the boss gameplay's fast forward, @0x53de84 / @0x53e000)
    void SetFastForward(bool b) { m_fastForward = b; }
    const std::list<GameplayTemplateInstance*> &GetLevelTemplateInstanceList() const { return m_instances; }
    const std::list<LevelTemplateData*> &GetLevelTemplateCache() const { return m_cache; }
    // the template that runs after or before this one (0 at the ends of the list; the current one comes before the list)
    GameplayTemplateInstance *GetNextLevelTemplateInstance(GameplayTemplateInstance*) const;
    GameplayTemplateInstance *GetPreviousLevelTemplateInstance(GameplayTemplateInstance*) const;
    const jet::String &GetLastBackgroundLocation() const;
    void GetNextBackgroundInfo(BackgroundInfo&) const;
    void OnCreateGameplayTemplateInstance();
    // every frame: the templates are made as the run needs them, the run goes on to the next one, they are shown or hidden, and the level
    // moves back to the origin when the minion is far from it
    void UpdateLevelParts();
    // the template ahead the sequence names takes the first place of the cache
    void IncludeInLevelSequenceFront(int templateId);
    // where a distance of the run is on the templates; the template that holds it
    GameplayTemplateInstance *GetPredictedTransform(unsigned distance, math::vec3<float> &pos, math::quat<float> &rot) const;
    // the template that holds the point gives the closest place of its lanes
    GameplayTemplateInstance *GetClosestLaneTransform(const math::vec3<float> &p, math::vec3<float> &pos, math::quat<float> &rot) const;
    void GetLastTemplateEndAnchor(math::vec3<float> &pos, float &angle);
    // whether the run may move back to the origin now, and does it (the whole level, with the minion, moves back by whole meters)
    bool CanCheckOriginDistance() const;
    void CheckOriginDistance();
    static const float k_originDistanceLimit;
private:
    // the minion went on to the next template: the one it left goes (to the level's deletion once behind) and the next one is made
    bool AdvanceLevelTemplateList();
    // a tee waits for the player's lane: once it is chosen the sequence goes on (true when a template was made)
    bool UpdateConditionalBifurcationState();
    bool UpdateTemplateInstanceBuilding();
    bool UpdateVisibilities();
    void CacheLevelSequence();
    void ConsumeCacheLevelSequence();
    GameplayTemplateInstance *BuildLevelPart(const jet::String &name);
    GameplayTemplateInstance *CreateGameplayTemplateInstance();
    static void ProcessTemplateDefFolder(LevelTemplateDef*, const clara::Folder&);
    LevelTemplateSequenceProvider *m_provider;           // 0x0
    std::vector<HistoryData> m_history;                  // 0x4
    std::list<LevelTemplateData*> m_cache;               // 0x10
    std::list<GameplayTemplateInstance*> m_instances;    // 0x18
    GameplayTemplateInstance *m_current;                 // 0x20
    bool m_secretExitTaken;                              // 0x24
    unsigned m_frameIndex;                               // 0x28: the frame the templates were last built in
    float m_distance;                                    // 0x2C: where the next template starts
    bool m_cacheStarted;                                 // 0x30
    bool m_bifurcationBlocked;                           // 0x31: a tee waits for the player's choice
    bool m_teeInCache;                                   // 0x32
    bool m_pausedCreation;                               // 0x33
    bool m_pauseInCache;                                 // 0x34
    int m_totalDistance;                                 // 0xC8
    bool m_fastForward;                                  // 0xCC
    static std::map<jet::String, LevelTemplateDef*> s_levelTemplates, s_backgroundTemplates;
    static bool s_areLibrariesProcessed;
    static const jet::String k__leftString, k__rightString, k__teeString, k__secretLeftString, k__secretRightString, k__specialString,
        k__MacholabSpecialLeft, k__MacholabSpecialRight;
};

#endif
