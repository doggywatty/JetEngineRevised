// libs/manhattan/include/dlc/tocparser.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_TOCPARSER_H
#define LIBS_MANHATTAN_INCLUDE_DLC_TOCPARSER_H

namespace manhattan::dlc {
class TOCParser {  // sizeof 0x1
public:
    static bool Parse(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::Value&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleVersion(const Json::Value&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetBundleRevision(const Json::Value&);
    static bool IsAssetMandatory(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool IsAssetOnDemand(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool IsAssetIndexed(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool IsAssetActive(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static int GetIndex(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static const Json::Value &GetVersionItems(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetAllAssetsInTOC(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetToBeRemovedAssets(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > GetAllOnDemandAssetsInTOC(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > GetOrderedToBeAutoDownloadedAssetsFromTOC(const Json::Value&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    TOCParser(const manhattan::dlc::TOCParser&);
    TOCParser();
    manhattan::dlc::TOCParser &operator=(const manhattan::dlc::TOCParser&);
    static manhattan::dlc::ITOCWalker *WalkerCreator(const Json::Value&);
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > VERSION_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > IRIS_ASSET_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > ASSET_SIZE_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > GAME_VERSIONS_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > METADATA_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > METADATA_INDEX_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > METADATA_MANDATORY_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > METADATA_ACTIVE_KEY;
};
}

#endif
