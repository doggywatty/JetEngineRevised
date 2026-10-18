// core/jet/include/dbg/debugcontext.h
#ifndef CORE_JET_INCLUDE_DBG_DEBUGCONTEXT_H
#define CORE_JET_INCLUDE_DBG_DEBUGCONTEXT_H

namespace dbg {
class DebugContext : public boost::noncopyable_::noncopyable {  // sizeof 0xA0
public:
    DebugContext();
    ~DebugContext();
    dbg::DebugContext::HeaderProxy &Header(const jet::String&);
    dbg::DebugContext::DataProxy &Data();
    void Child(unsigned);
    const jet::String &GetName();
    const unsigned GetId();
    void SetEnabled(bool);
    bool IsEnabled();
    const std::vector<dbg::DebugContext::DebugHeader,std::allocator<dbg::DebugContext::DebugHeader> > &GetHeaders();
    void SetEngine(bool);
    bool IsEngine();
    void AddHeader(const dbg::DebugContext::Checkbox&);
    void AddHeader(const dbg::DebugTypeDef::type&);
    void AddHeader(const bool&);
    void AddHeader(const jet::String&);
    void AddHeader(const char*);
    void AddHeader(const math::vec3<float>&);
    void AddHeader(const math::vec2<float>&);
    void AddHeader(const double&);
    void AddHeader(const float&);
    void AddHeader(const unsigned __int64&);
    void AddHeader(const __int64&);
    void AddHeader(const unsigned&);
    void AddHeader(const int&);
    void AddHeader(const unsigned short&);
    void AddHeader(const short&);
    void AddHeader(const unsigned char&);
    void AddHeader(const signed char&);
    void AddData(safe_enum<dbg::DebugTypeDef,enum dbg::DebugTypeDef::type>, const char*);
    void AddData(const dbg::DebugContext::Checkbox&);
    void AddData(const bool&);
    void AddData(const jet::String&);
    void AddData(const char*);
    void AddData(const math::vec3<float>&);
    void AddData(const math::vec2<float>&);
    void AddData(const double&);
    void AddData(const float&);
    void AddData(const unsigned __int64&);
    void AddData(const __int64&);
    void AddData(const unsigned&);
    void AddData(const int&);
    void AddData(const unsigned short&);
    void AddData(const short&);
    void AddData(const unsigned char&);
    void AddData(const signed char&);
    void AddCommand(const jet::String&, const boost::function<void __cdecl(void *)>&);
    void GetCommandList(std::vector<jet::String,std::allocator<jet::String> >&);
    void SetItemChecked(unsigned __int64, bool);
    bool IsItemChecked(unsigned __int64);
    void ClearCheckedItems();
protected:
    void StartFrame();
    void EndFrame();
    jet::stream::MemoryStream &GetData();
private:
    bool m_enabled;  // 0x0
    unsigned short m_headerCount;  // 0x2
    unsigned short m_dataCount;  // 0x4
    unsigned m_binIndex;  // 0x8
    jet::String m_binName;  // 0xC
    dbg::DebugContext::HeaderProxy m_header;  // 0x10
    dbg::DebugContext::DataProxy m_data;  // 0x14
    dbg::DebugContext::ChildProxy m_child;  // 0x1C
    bool m_engineCtx;  // 0x20
    std::vector<dbg::DebugContext::DebugHeader,std::allocator<dbg::DebugContext::DebugHeader> > m_headers;  // 0x24
    std::map<jet::String,boost::function<void __cdecl(void *)>,std::less<jet::String>,std::allocator<std::pair<jet::String const ,boost::function<void __cdecl(void *)> > > > m_cmdMap;  // 0x38
    boost::unordered::unordered_set<unsigned __int64,boost::hash<unsigned __int64>,std::equal_to<unsigned __int64>,std::allocator<unsigned __int64> > m_uncheckedItems;  // 0x54
    jet::stream::MemoryStream m_stream;  // 0x70
};
}

#endif
