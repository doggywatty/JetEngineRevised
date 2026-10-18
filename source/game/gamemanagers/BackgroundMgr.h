// source/game/gamemanagers/BackgroundMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_BACKGROUNDMGR_H
#define SOURCE_GAME_GAMEMANAGERS_BACKGROUNDMGR_H

#include <list>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "smath.h"

class BackgroundTemplateInstance;
class GameEntity;
namespace clara { class DataEntity; }

// The backgrounds around the run: a pool of them is kept ahead of the minion, each made from a Background_ library, and the
// ones behind are dropped. In the fluffy dimension and the large minion mode other pools take over.
class BackgroundMgr : public Singleton<BackgroundMgr> {  // sizeof 0x114
public:
    // the backgrounds of one pool and where that pool's world starts
    struct BackgroundList {
        std::list<BackgroundTemplateInstance*> items;
        math::vec3<float> origin;
    };
    explicit BackgroundMgr(bool enabled);
    virtual ~BackgroundMgr();
    void PostInit();
    void Update();
    void Render();
    void BuildBackgroundPool() { if (m_enableBackground) InitBackgrounds(); }
    void DropBackgrounds(unsigned distance);
    void Translate(const math::vec3<float>&);
    // makes the backgrounds of a new gameplay template instance: one every k_backgroundTemplateSize meters, or at its turns
    void OnCreateGameplayTemplateInstance();
    void EnableFluffyDimensionBackground(bool);
    void EnableLargeMinionBackground(bool);
    int GetLocation() const { return m_location; }
    int GetLocationAtDistance(unsigned distance) const;
    int GetLocationOfCurrentBackground() const;
    const jet::String &GetCurrentLocationTitle() const;
    std::vector<int> GetLocationPoolArray() const;
    void ApplySkyDome();
    void ApplyFogValue();
    void ApplyLensFlare();
    void ApplyCurrentBackgroundSound();
private:
    // where a background was and what it applied, to put it back after the large minion mode
    struct BackupBackground {
        jet::String name;
        math::vec3<float> position;
        float angle;
        math::vec3<float> direction;
        unsigned applyDistance, removeDistance;
    };
    void InitBackgrounds();
    void ResetBackground();
    void ResetFluffyBackground();
    jet::String GetBGName(const BackgroundTemplateInstance*) const;
    jet::String GetBGName(const jet::String &variant) const { return k_backgroundPrefix + variant; }
    BackgroundTemplateInstance *GetNextBG(const jet::String &name);
    void DropBG(BackgroundTemplateInstance*);
    void GrabBG(BackgroundTemplateInstance*);
    void ApplyBackgroundSettings(const BackgroundTemplateInstance*);
    void UpdateBackgroundPool();
    void UpdateFluffyBackgroundPool();
    void CheckSettingsChange();
    bool ShouldApplySettings(const BackgroundTemplateInstance*) const;
    void BackUp(std::vector<BackupBackground>&);
    void Restore(std::vector<BackupBackground>&);
    void ProcessCreatedEntities(BackgroundTemplateInstance*);
    jet::String NextFluffyBackgroundName() const;
public:
    static const jet::String k_backgroundPrefix, k_leftString, k_rightString, k_teeString, k_emptyString;
    static const float k_backgroundTemplateSize, k_safeBackgroundChangeOffset, k_hidenPositionOffset;
    static const unsigned k_backgroundCount;
private:
    float m_changeOffset;                  // 0x8
    int m_checkOffset;                     // 0xC
    BackgroundList m_lists[3];             // 0x10 the run, 0x24 the fluffy dimension (at the fluffy gameplay's place), 0x38 the large minion
    BackgroundList *m_currentList;         // 0x4C
    const BackgroundTemplateInstance *m_lastApplied;  // 0x50
    int m_location, m_currentLocation;     // 0x54, 0x58
    clara::DataEntity *m_fluffyBackgrounds;  // the fluffy gameplay's pick of backgrounds for the location (FluffyGameplay +0x8C)
    std::list<GameEntity*> m_bgEntities;   // 0x5C
    unsigned m_createdSizeBackup;          // 0x64
    unsigned m_createdSize;                // 0x68
    std::vector<BackupBackground> m_backup;  // 0x6C
    unsigned m_updateFrame, m_dropFrame;   // 0x78, 0x7C
    bool m_enableBackground;               // 0x110
};

#endif
