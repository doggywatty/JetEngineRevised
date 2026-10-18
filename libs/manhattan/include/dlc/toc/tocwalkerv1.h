// libs/manhattan/include/dlc/toc/tocwalkerv1.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_TOC_TOCWALKERV1_H
#define LIBS_MANHATTAN_INCLUDE_DLC_TOC_TOCWALKERV1_H

namespace manhattan::dlc {
class TOCWalkerV1 : public manhattan::dlc::ITOCWalker {  // sizeof 0x4
public:
    TOCWalkerV1();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleVersion(const Json::Value&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleRevision(const Json::Value&);
    virtual const Json::Value &GetVersionItems(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
};
}

#endif
