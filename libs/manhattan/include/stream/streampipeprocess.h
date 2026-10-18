// libs/manhattan/include/stream/streampipeprocess.h
#ifndef LIBS_MANHATTAN_INCLUDE_STREAM_STREAMPIPEPROCESS_H
#define LIBS_MANHATTAN_INCLUDE_STREAM_STREAMPIPEPROCESS_H

namespace manhattan::stream {
class StreamPipeProcess {  // sizeof 0xC
public:
    StreamPipeProcess(manhattan::stream::IStreamW*, manhattan::stream::IStreamW*);
    virtual ~StreamPipeProcess();
    virtual int Execute(unsigned char*, int, unsigned char*, int) = 0;
protected:
    manhattan::stream::IStreamW *m_input;  // 0x4
    manhattan::stream::IStreamW *m_output;  // 0x8
};
}

#endif
