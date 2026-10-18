// libs/manhattan/include/dlc/toc/tocwalkerv2.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_TOC_TOCWALKERV2_H
#define LIBS_MANHATTAN_INCLUDE_DLC_TOC_TOCWALKERV2_H

namespace manhattan::dlc {
class TOCWalkerV2 : public manhattan::dlc::ITOCWalker {  // sizeof 0x4
public:
    TOCWalkerV2();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleVersion(const Json::Value&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleRevision(const Json::Value&);
    virtual const Json::Value &GetVersionItems(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > BUNDLE_VERSION_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > BUNDLE_REVISION_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > GAME_VERSIONS_KEY;
};
}

#endif
