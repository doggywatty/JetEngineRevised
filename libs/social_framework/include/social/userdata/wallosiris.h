// libs/social_framework/include/social/userdata/wallosiris.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_WALLOSIRIS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_WALLOSIRIS_H

namespace social {
class WallOsiris : public social::Wall {  // sizeof 0x14C
public:
    WallOsiris(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    WallOsiris(const social::UserOsiris*);
    virtual ~WallOsiris();
    int GetNumPosts();
    const social::WallOsiris::SWallPostOsiris &operator[](unsigned);
    social::WallOsiris::SWallPostOsiris &operator[](unsigned);
    const social::WallOsiris::SWallPostOsiris &GetPost(unsigned);
    social::WallOsiris::SWallPostOsiris &GetPost(unsigned);
protected:
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
    virtual void SavePost(const social::Wall::SWallPost&);
private:
    static void sOnPosted(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    static void sOnRead(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    gaia::Osiris::OsirisWallPostObjectType m_osirisObjectType;  // 0xDC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallId;  // 0xE0
    std::vector<social::WallOsiris::SWallPostOsiris,std::allocator<social::WallOsiris::SWallPostOsiris> > m_wallPosts;  // 0x100
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_jsonWallAnswer;  // 0x114
    social::WallOsiris::SWallPostOsiris m_defaultPost;  // 0x128
};
}

#endif
