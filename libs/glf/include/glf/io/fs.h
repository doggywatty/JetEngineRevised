// libs/glf/include/glf/io/fs.h
#ifndef LIBS_GLF_INCLUDE_GLF_IO_FS_H
#define LIBS_GLF_INCLUDE_GLF_IO_FS_H

namespace glf {
class Fs : private glf::NonCopyable, public glf::ios {  // sizeof 0x5F0
public:
    static glf::Fs &Get();
    static glf::Fs *Inst();
    void Win32OverrideDataPath(const char*);
    void IphoneOverrideDataPath(const char*);
    void MacOSXOverrideDataPath(const char*);
    void ClearHomeDataDir();
    void UpdateHomeDataDir();
    void _CopyDataToHomeData(glf::fs::DirHandle&);
    bool MoveFileA(const char*, const char*);
    bool RemoveFile(const char*);
    bool CopyFile(const char*, glf::ios::openflag, const char*, glf::ios::openflag);
    bool DoesFileExist(const char*, glf::ios::openflag);
    bool DoesFileExist(const char*);
    bool MkDir(const char*, glf::ios::openflag);
    bool RmDir(const char*, glf::ios::openflag);
    bool EmptyDir(const char*, glf::ios::openflag);
    bool GetInfo(const char*, glf::fs::FileInfo&, glf::ios::openflag);
    glf::FsError GetLastError();
    glf::ios::openflag ResolvePath(const char*, glf::ios::openflag, char*, unsigned);
    bool MountDrive(const char*, const char*, glf::ios::openflag);
    bool UnmountDrive(const char*);
    const char *GetMountedDrive(const char*);
    const char *GetDir(glf::ios::openflag);
    void SetDir(glf::ios::openflag, const char*);
    const char *GetDataDir();
    const char *GetHomeDir();
    const char *GetTmpDir();
    void SetDataDir(const char*);
    void SetHomeDir(const char*);
    void SetTempDir(const char*);
    bool LoadObfuscationMap(const char*, unsigned char);
    bool IsForcingLowercase();
    void SetIsForcingLowercase(bool);
private:
    Fs();
    ~Fs();
public:
    glf::Fs::Impl &GetImpl();
private:
    static glf::Fs *sFs;
    bool mIsForcingLowercase;  // 0x1
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mDataDir;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mHomeDir;  // 0x24
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mTmpDir;  // 0x44
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mBakDir;  // 0x64
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mRawDir;  // 0x84
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mNocopyDir;  // 0xA4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mCwdDir;  // 0xC4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mCwdStack[4];  // 0xE4
    unsigned mCwdStackIdx;  // 0x164
    glf::Fs::Link mLinks[16];  // 0x168
    unsigned mNumLinked;  // 0x5A8
    glf::Fs::Link *mLinked[16];  // 0x5AC
    glf::Fs::Impl &m;  // 0x5EC
};
}

#endif
