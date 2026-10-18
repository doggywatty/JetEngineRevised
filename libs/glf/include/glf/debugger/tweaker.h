// libs/glf/include/glf/debugger/tweaker.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUGGER_TWEAKER_H
#define LIBS_GLF_INCLUDE_GLF_DEBUGGER_TWEAKER_H

namespace glf::debugger {
class Tweakable {  // sizeof 0xA8
public:
    Tweakable();
    virtual ~Tweakable();
    const char *GetClassName();
    glf::debugger::Tweakable::Type GetType(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual const char *GetInstanceName();
    virtual void WriteValues(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void LoadXML(glf::IOStream&, bool);
    void LoadXML(const char*, bool);
    void SaveXML(const char*);
private:
    void SetValuesFromXML(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
public:
    virtual void OnSetValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetClassName(const char*);
    void RegisterVariable(const char*, glf::debugger::Tweakable::Vector4d&);
    void RegisterVariable(const char*, glf::debugger::Tweakable::Vector3d&);
    void RegisterVariable(const char*, glf::debugger::Tweakable::Vector2d&);
    void RegisterVariable(const char*, glf::debugger::Tweakable::Colorf&);
    void RegisterVariable(const char*, glf::debugger::Tweakable::Color&);
    void RegisterVariable(const char*, bool&);
    void RegisterVariable(const char*, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void RegisterVariable(const char*, float&);
    void RegisterVariable(const char*, int&);
    void SetRangeDesc(const char*, const char*, const char*);
    void SetRange(const char*, const char*);
    void SetDesc(const char*, const char*);
    void PushGroup(const char*);
    void PopGroup();
    void ClearGroups();
    void UnregisterVariable(const char*);
    void ClearGroup(std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >);
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetCurrentGroupPath();
    bool SelectGroup(const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    void AddColumn();
    void SendValues();
private:
    void SetValue(glf::debugger::Tweakable::Group&, glf::debugger::Tweakable::Type, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void WriteGroup(glf::debugger::Tweakable::Group&, glf::debugger::XmlWriter&, bool);
    glf::debugger::Tweakable::Group *GetCurrentGroup();
    void RegisterVariableName(const char*);
    glf::debugger::Tweakable::Group mBaseGroup;  // 0x4
    glf::debugger::Tweakable::Group *mCurrentGroup;  // 0x6C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mClassName;  // 0x70
    std::vector<char,std::allocator<char> > mXml;  // 0x90
    int mColumnVar;  // 0xA4
};
}

namespace glf::debugger {
class Tweakers : public glf::debugger::Module, public glf::SingletonWithDep<glf::debugger::Tweakers,glf::debugger::Debugger> {  // sizeof 0xA4
public:
    virtual ~Tweakers();
    virtual void UpdateFrame();
    virtual void Parse(int, glf::debugger::PacketReader&);
    virtual void ConnectionClosed();
private:
    Tweakers();
    void RegisterTweakable(glf::debugger::Tweakable*);
    void UnregisterTweakable(glf::debugger::Tweakable*);
    void SendTweakables();
    void SendValues(glf::debugger::Tweakable*);
    std::map<glf::debugger::Tweakable *,bool,std::less<glf::debugger::Tweakable *>,std::allocator<std::pair<glf::debugger::Tweakable * const,bool> > > mTweakables;  // 0x50
    std::map<glf::debugger::Tweakable *,bool,std::less<glf::debugger::Tweakable *>,std::allocator<std::pair<glf::debugger::Tweakable * const,bool> > > mAddedTweakables;  // 0x6C
    std::map<glf::debugger::Tweakable *,bool,std::less<glf::debugger::Tweakable *>,std::allocator<std::pair<glf::debugger::Tweakable * const,bool> > > mRemovedTweakables;  // 0x88
};
}

namespace glf::debugger {
class XmlHandler : public glf::debugger::XmlReader::Handler {  // sizeof 0x84
public:
    XmlHandler(glf::debugger::Tweakable*);
    virtual void BeginTag(std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    virtual void EndTag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    glf::debugger::Tweakable::Group mDummyGroup;  // 0x4
    std::vector<glf::debugger::Tweakable::Group *,std::allocator<glf::debugger::Tweakable::Group *> > mStack;  // 0x6C
    glf::debugger::Tweakable *mTweakable;  // 0x80
};
}

#endif
