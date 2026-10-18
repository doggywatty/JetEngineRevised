// libs/vox1.1/include/vox_zip_reader.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_ZIP_READER_H
#define LIBS_VOX1_1_INCLUDE_VOX_ZIP_READER_H

namespace vox {
class CZipReader : public vox::FileArchive {  // sizeof 0x4C
public:
    CZipReader(const char*, bool, bool);
    virtual ~CZipReader();
    virtual bool getFileInfo(const char*, int&, int&);
    virtual const char *getZipFileName();
    virtual bool isValid();
    virtual int getFileCount();
    virtual bool isVoxArchive();
    bool hasExtensionTable();
    bool ExportHeader(vox::ZipTableSerializer&);
private:
    bool scanLocalHeader();
    bool ImportHeader(vox::ZipTableSerializer&);
protected:
    void extractFilename(vox::SZipFileEntry*);
    void deletePathFromFilename(std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >&);
private:
    vox::FileInterface *File;  // 0x4
    std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > m_archiveName;  // 0x8
protected:
    bool IgnoreCase;  // 0x28
    bool IgnorePaths;  // 0x29
    std::map<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> >,vox::SZipFileEntry,vox::StringComp,vox::SAllocator<std::pair<std::basic_string<char,std::char_traits<char>,vox::SAllocator<char,0> > const ,vox::SZipFileEntry>,0> > FileList;  // 0x2C
    bool m_isValid;  // 0x48
    bool m_hasExtensionTable;  // 0x49
};
}

#endif
