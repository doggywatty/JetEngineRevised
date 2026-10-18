// libs/manhattan/include/dlc/toc/itocwalker.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_TOC_ITOCWALKER_H
#define LIBS_MANHATTAN_INCLUDE_DLC_TOC_ITOCWALKER_H

namespace manhattan::dlc {
class ITOCWalker {  // sizeof 0x4
public:
    virtual ~ITOCWalker();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleVersion(const Json::Value&) = 0;
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleRevision(const Json::Value&) = 0;
    virtual const Json::Value &GetVersionItems(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&) = 0;
};
}

#endif
