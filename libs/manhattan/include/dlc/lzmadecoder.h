// libs/manhattan/include/dlc/lzmadecoder.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_LZMADECODER_H
#define LIBS_MANHATTAN_INCLUDE_DLC_LZMADECODER_H

namespace manhattan::dlc {
class LzmaDecoder : public manhattan::dlc::DetailsHandler<manhattan::dlc::LzmaDecoderTaskDetails> {  // sizeof 0xB8
public:
    LzmaDecoder(manhattan::dlc::DetailsHandler<manhattan::dlc::LzmaDecoderTaskDetails>&, manhattan::dlc::BaseWriter*);
    LzmaDecoder();
    virtual ~LzmaDecoder();
    int ExecuteStepBase(unsigned char*, unsigned, unsigned char*, int, unsigned&, unsigned&);
    int SetupStep(manhattan::dlc::LzmaDecoder::LzmaWorkBuffer*, manhattan::dlc::LzmaDecoder::LzmaWorkBuffer*);
    void ExecuteStep();
    bool StepCompleted();
    bool IsDecoderRunning();
    int GetLzmaInternalErrorCode();
    manhattan::dlc::BaseWriter &GetWriter();
    static unsigned __int64 GetUnpackedFileSize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    manhattan::dlc::LzmaDecoder::LzmaWorkBuffer *in;  // 0xC
    manhattan::dlc::LzmaDecoder::LzmaWorkBuffer *out;  // 0x10
    unsigned m_totalBytesIn;  // 0x14
    int m_DecMode;  // 0x18
    ISzAlloc m_alloc;  // 0x1C
    static const unsigned m_kHeaderSize;
    unsigned char m_header[13];  // 0x24
    unsigned __int64 m_unpackSize;  // 0x38
    CLzmaDec m_state;  // 0x40
    manhattan::dlc::BaseWriter *m_writer;  // 0xB0
};
}

#endif
