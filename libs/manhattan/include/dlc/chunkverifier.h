// libs/manhattan/include/dlc/chunkverifier.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_CHUNKVERIFIER_H
#define LIBS_MANHATTAN_INCLUDE_DLC_CHUNKVERIFIER_H

namespace manhattan::dlc {
class ChunkVerifier {  // sizeof 0x48
public:
    ChunkVerifier(manhattan::dlc::SharedFile&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value*);
    virtual ~ChunkVerifier();
    bool IsCompleted();
    bool IsVerified();
    bool IsLastData(int, unsigned);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSHA1Digest(unsigned char*, unsigned);
    bool CheckChunk(unsigned char*, unsigned, int);
    unsigned GetChunkSize();
    unsigned GetLastValidChunk();
    void Update();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_asset;  // 0x4
    manhattan::dlc::SharedFile &m_sharedFile;  // 0x24
    manhattan::dlc::SharedFilePointer m_sfp;  // 0x28
    const Json::Value *m_hashData;  // 0x34
    int m_lastVerifiedChunk;  // 0x38
    unsigned m_chunkSize;  // 0x3C
    unsigned char* m_buffer;  // 0x40
    bool m_completed;  // 0x44
    bool m_verification;  // 0x45
};
}

#endif
