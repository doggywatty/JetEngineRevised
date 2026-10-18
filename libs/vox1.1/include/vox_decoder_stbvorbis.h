// libs/vox1.1/include/vox_decoder_stbvorbis.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_DECODER_STBVORBIS_H
#define LIBS_VOX1_1_INCLUDE_VOX_DECODER_STBVORBIS_H

namespace vox {
class DecoderStbVorbis : public vox::DecoderInterface {  // sizeof 0xC
public:
    DecoderStbVorbis();
    virtual ~DecoderStbVorbis();
    virtual vox::DecoderCursorInterface *CreateNewCursor(vox::StreamCursorInterface*);
    virtual void DestroyCursor(vox::DecoderCursorInterface*);
    virtual int GetType();
    virtual void* GetParam();
    stb_vorbis *GetVorbisFile();
    void SetVorbisFile(stb_vorbis*);
    unsigned GetStreamOffset();
    void SetStreamOffset(unsigned);
private:
    stb_vorbis *m_pVorbisFile;  // 0x4
    unsigned m_streamOffset;  // 0x8
};
}

namespace vox {
class DecoderStbVorbisCursor : public vox::DecoderCursorInterface {  // sizeof 0x2C
protected:
    DecoderStbVorbisCursor(vox::DecoderInterface*, vox::StreamCursorInterface*);
public:
    virtual ~DecoderStbVorbisCursor();
    virtual int Decode(void*, int);
    virtual int Seek(unsigned);
    virtual bool HasData();
protected:
    stb_vorbis *m_pVorbisFile;  // 0x20
    unsigned m_totalSampleDecoded;  // 0x24
    bool m_isDecoderInError;  // 0x28
};
}

#endif
