// source/game/interfaces/aurora/AuroraLevelData.h
#ifndef SOURCE_GAME_INTERFACES_AURORA_AURORALEVELDATA_H
#define SOURCE_GAME_INTERFACES_AURORA_AURORALEVELDATA_H

#include <map>
#include <string>
#include <vector>
#include "core/pstring.h"
#include "smath.h"
#include "stream/istream.h"

// One GUI layer of the Aurora editor: a list of elements, each a list of 16 bit parameters
// (type, unique id, x, y, anchors, ... see re/NOTES.md), kept as authored and rescaled to the display
class AuroraLevelData {  // sizeof 0x54
public:
    static const jet::String &GetAuroraStringFromIndex(int);
    AuroraLevelData();
    ~AuroraLevelData();
    void ResetGUIToDisplayData(float gameW, float gameH, float scale);
    void Unload();
    void Load(jet::stream::IStream&);
    void LoadElementNames(jet::stream::IStream&);
    int GetElementWidthParameterId(unsigned);
    int GetElementHeightParameterId(unsigned);
    int GetElementParentId(unsigned);
    math::vec2<float> GetDisplaySize() const { return m_displaySize; }
    const jet::String &GetLevelName() const { return m_layerName; }
    const jet::String &GetElementName(unsigned i) const { return m_layerElemNames[i]; }
    unsigned GetElementsCount() const { return m_elementsCount; }
    int GetElementIndex(const jet::String&);
    int GetUniqueIdElemPos(unsigned);
    const jet::String &GetUniqueIdElemName(unsigned);
    int GetElementParamCount(unsigned) const;
    int GetParamValue(int elem, int param) const { return m_gameDataRescaled[elem][param]; }
    int GetInitialParamValue(unsigned elem, unsigned param) const { return m_gameDataInitial[elem][param]; }
private:
    void _SetParamValue(int elem, int param, int v) { m_gameDataRescaled[elem][param] = (short)v; }
    void _RescaleAll();
    int _PostInitGetAnchorSnapId(unsigned);
    void _PostInitElementUpperLeftAnchorScreen(unsigned);
    void _PostInitElementUpperLeftAnchorSnap(unsigned);
    void _PostInitElementLowerRightAnchorScreen(unsigned);
    void _PostInitElementLowerRightAnchorSnap(unsigned);
    math::vec2<float> _GetGameFileAnchorOffset(int);
    math::vec2<float> _GetDisplayAnchorOffset(int);
    math::vec2<float> _GetAnchorOffset(int, math::vec2<float>);
    int AddString(const jet::String&);
    static const jet::String MISSING_AURORA_STR;
    static std::map<std::string, int> s_auroraStringMap;
    static std::vector<jet::String> s_auroraStrings;
    math::vec2<float> m_gameFileScreenSize;                 // 0x0
    math::vec2<float> m_displaySize;                        // 0x8
    math::vec2<float> m_aspectRatioScale;                   // 0x10
    float m_scale;                                          // 0x18
    std::vector<std::vector<short> > m_gameDataInitial;     // 0x1c
    std::vector<std::vector<short> > m_gameDataRescaled;    // 0x28
    unsigned m_elementsCount;                               // 0x34
    std::vector<short> m_gameDataRescaleFlags;              // 0x38
    jet::String m_layerName;                                // 0x44
    std::vector<jet::String> m_layerElemNames;              // 0x48
};

#endif
