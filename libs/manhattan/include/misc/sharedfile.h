// libs/manhattan/include/misc/sharedfile.h
#ifndef LIBS_MANHATTAN_INCLUDE_MISC_SHAREDFILE_H
#define LIBS_MANHATTAN_INCLUDE_MISC_SHAREDFILE_H

namespace manhattan::dlc {
class SharedFile {  // sizeof 0x3C
    void Open(bool);
public:
    SharedFile(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    SharedFile();
    virtual ~SharedFile();
    void Close();
    manhattan::dlc::ESharedFileState GetState();
    void SetState(manhattan::dlc::ESharedFileState);
    unsigned Write(manhattan::dlc::SharedFilePointer&, const void*, unsigned);
    unsigned Read(manhattan::dlc::SharedFilePointer&, void*, unsigned);
    bool NonGreedyRead(manhattan::dlc::SharedFilePointer&, unsigned char*, unsigned, unsigned, unsigned&);
    bool Preallocate(long);
    void NotifyAllDataWritten();
    void NotifyAllDataRead();
    bool IsDataProcessingFinished();
    bool IsDataWritingFinished();
    void AcknowledgeFileCompletion();
    unsigned GetAllocatedSize();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFilename();
private:
    long m_allocatedSize;  // 0x4
    long m_maxPosAvailableData;  // 0x8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_fileName;  // 0xC
    _iobuf *m_file;  // 0x2C
    manhattan::dlc::ESharedFileState m_state;  // 0x30
    glwebtools::Mutex m_mutex;  // 0x34
};
}

#endif
