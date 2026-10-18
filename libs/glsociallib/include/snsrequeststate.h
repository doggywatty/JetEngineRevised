// libs/glsociallib/include/snsrequeststate.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_SNSREQUESTSTATE_H
#define LIBS_GLSOCIALLIB_INCLUDE_SNSREQUESTSTATE_H

namespace sociallib {
class SNSRequestState {  // sizeof 0x108
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > static_initSNSNames();
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > static_initSNSRequestTypeNames();
public:
    SNSRequestState(sociallib::ClientSNSEnum, void sociallib::SNSWrapperBase::(sociallib::SNSRequestState&) sociallib::SNSWrapperBase::*, sociallib::SNSRequestTypeEnum, sociallib::SNSRequestDataTypeEnum, sociallib::SNSRequestStateEnum);
    ~SNSRequestState();
    int getParamListSize();
    int getParamType();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > getStringParam();
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > getStringArrayParam();
    int getIntParam();
    unsigned getUnsignedIntParam();
    __int64 getInt64Param();
    bool getBoolParam();
    void writeParamListSize(int);
    void writeStringParam(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void writeStringArrayParam(const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    void writeIntParam(int);
    void writeUnsignedIntParam(unsigned);
    void writeInt64Param(__int64);
    void writeBoolParam(bool);
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > s_snsNames;
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > s_snsRequestTypeNames;
    sociallib::SNSRequestStateEnum m_currentState;  // 0x0
    sociallib::SNSRequestTypeEnum m_requestType;  // 0x4
    sociallib::SNSRequestDataTypeEnum m_dataType;  // 0x8
    sociallib::ClientSNSEnum m_snsId;  // 0xC
    bool m_isCanceled;  // 0x10
    void (*m_executeFunc)(sociallib::SNSRequestState&);  // 0x18
    sociallib::CDynamicMemoryStream m_paramList;  // 0x28
    bool m_boolData;  // 0x3C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_strData;  // 0x40
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_strArrayData;  // 0x60
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_strMapData;  // 0x74
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,sociallib::SNSUserData,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,sociallib::SNSUserData> > > m_userDataMap;  // 0x90
    std::vector<sociallib::SNSAchievementData,std::allocator<sociallib::SNSAchievementData> > m_achievementData;  // 0xAC
    std::vector<sociallib::SNSLeaderboardRowData,std::allocator<sociallib::SNSLeaderboardRowData> > m_leaderboardRowData;  // 0xC0
    std::vector<sociallib::SNSLeaderboard,std::allocator<sociallib::SNSLeaderboard> > m_LeaderboardsType;  // 0xD4
    sociallib::CDynamicMemoryStream m_byteBufferData;  // 0xE8
    int m_intData;  // 0xFC
    bool m_isDialogRequest;  // 0x100
    bool m_dialogCancelByUser;  // 0x101
    int m_priority;  // 0x104
};
}

#endif
