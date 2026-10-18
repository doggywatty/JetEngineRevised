// libs/social_framework/include/social/userdata/wall.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_WALL_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_WALL_H

namespace social {
class Wall : public social::Storable {  // sizeof 0xDC
public:
    Wall(social::Wall::WallType);
    virtual ~Wall();
    void Post(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Post(const social::Wall::SWallPost&);
    social::Wall::WallType GetWallType();
protected:
    virtual void SaveImpl();
    void OnPostSaved(bool);
    virtual void SavePost(const social::Wall::SWallPost&) = 0;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    social::Wall::WallType m_wallType;  // 0xBC
    std::queue<social::Wall::SWallPost,std::deque<social::Wall::SWallPost,std::allocator<social::Wall::SWallPost> > > m_wallPostQueue;  // 0xC0
};
}

#endif
