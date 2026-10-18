// libs/gaia/include/utils/securestoragemanager.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_SECURESTORAGEMANAGER_H
#define LIBS_GAIA_INCLUDE_UTILS_SECURESTORAGEMANAGER_H

class SecureStorageManager {  // sizeof 0x20
    SecureStorageManager();
public:
    static SecureStorageManager *GetInstance();
    StorageErrors SaveValueToUserDefaults(const char*, const char*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetValueFromUserDefaults(const char*);
    StorageErrors DeleteValueFromUserDefaults(const char*);
    StorageErrors SaveValueToLocalKeychain(const char*, const char*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetValueFromLocalKeychain(const char*);
    StorageErrors DeleteValueFromLocalKeychain(const char*);
    StorageErrors SaveValueToSharedKeychain(const char*, const char*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetValueFromSharedKeychain(const char*);
    StorageErrors DeleteValueFromSharedKeychain(const char*);
private:
    static SecureStorageManager *s_instance;
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_keychain_access_group;  // 0x0
};

#endif
