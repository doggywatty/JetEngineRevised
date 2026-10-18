// libs/gaia/include/utils/gameloftid.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_GAMELOFTID_H
#define LIBS_GAIA_INCLUDE_UTILS_GAMELOFTID_H

namespace gaia {
class GameloftID {  // sizeof 0x1
    static gaia::GLUID *GetLocalDeviceGLUID();
    static bool GLUIDDataExistsInOldLocations();
    static bool GLUIDExistsInNewLocations();
    static gaia::GLUID *GetGluidFromOldLocations();
    static gaia::GLUID *GetGluidFromNewLocations();
    static void LogGLIDUtils(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > EncryptAndEncodeIDData(const void*, const unsigned, const unsigned*);
    static void* DecodeAndDecryptIDData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const unsigned*);
    static bool EncryptAndSaveGLUIDToNewLocations(gaia::GLUID*);
    static bool DeleteGLUIDFromOldLocations();
    static gaia::GLUID *CreateNewGLUID();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GenerateRandomPassword();
public:
    static void ResetDeviceGLUID();
    static gaia::GLUID RetrieveDeviceGLUID();
    static gaia::DeviceInfo RetrieveDeviceInfo();
};
}

#endif
