// libs/gaia/include/crm/crmaction.h
#ifndef LIBS_GAIA_INCLUDE_CRM_CRMACTION_H
#define LIBS_GAIA_INCLUDE_CRM_CRMACTION_H

namespace gaia {
class CrmAction : public gaia::GaiaSimpleEventDispatcher<Json::Value> {  // sizeof 0xC8
    unsigned long GetUNIXSeconds();
public:
    CrmAction();
    int Initialize(const Json::Value&);
    int Trigger(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value&);
    Json::Value Serialize();
    int Deserialize(Json::Value);
    ~CrmAction();
    int Update(unsigned);
    int CheckCooldowns();
    int CheckTriggerConditions(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value&);
    bool CheckMathConditions(const Json::Value&, const Json::Value&);
    bool Evaluate(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value&);
    gaia::CrmAction::CrmActionType GetActionType();
    Json::Value &GetTypeSpecificFields();
private:
    gaia::CrmAction::CrmActionType m_type;  // 0x20
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_typeString;  // 0x24
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_id;  // 0x44
    double m_frequency;  // 0x68
    Json::Value m_typeSpecificFields;  // 0x70
    Json::Value m_pointcutTriggers;  // 0x80
    std::vector<gaia::CrmAction::CrmImpressionLimit,std::allocator<gaia::CrmAction::CrmImpressionLimit> > m_impressionLimits;  // 0x90
    std::queue<int,std::deque<int,std::allocator<int> > > m_currentImpressions;  // 0xA4
    bool m_mustBeTriggered;  // 0xC0
};
}

#endif
