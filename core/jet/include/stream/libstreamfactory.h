// core/jet/include/stream/libstreamfactory.h
#ifndef CORE_JET_INCLUDE_STREAM_LIBSTREAMFACTORY_H
#define CORE_JET_INCLUDE_STREAM_LIBSTREAMFACTORY_H

#include <memory>
#include <unordered_map>
#include "stream/istreamfactory.h"
#include "thread/mutex.h"

namespace jet {
namespace stream {

class IFileSystem;

// Gameloft ".lib" archive, mapped whole:
//   u32 count; u32 offset[count] (absolute); count x (u16 len, name); u8 padding[count]; data...
// Entry i spans offset[i] .. offset[i+1] (or end of file) minus padding[i].
class LibStreamFactory : public IStreamFactory {
public:
    LibStreamFactory(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext);
    LibStreamFactory(const String& path, const String& ext);
    LibStreamFactory(IStream* stream, const String& path, const String& ext);  // takes ownership
    virtual ~LibStreamFactory();

    virtual IStream* CreateStream(const String& name);
    virtual void RefreshStreams() {}
    virtual std::vector<String> GetFileList(const String& ext) const;
    virtual unsigned GetFileCount() const { return m_chunkCount; }
    virtual String GetPath() const { return m_path; }
    virtual String GetExtension() const { return m_extension; }
    virtual bool CanCreateStream(const String& name) const;
    bool IsOpen() const { return m_mappedMemory != nullptr; }

private:
    void Init();

    String m_path;
    String m_extension;
    IStream* m_stream;
    u8* m_mappedMemory;
    unsigned m_chunkCount;
    std::vector<u32> m_offsets;
    std::vector<u8> m_padding;
    std::unordered_map<String, unsigned> m_filenameToStreamIdxMap;
    mutable thread::RecursiveMutex m_filenameToStreamIdxMapMutex;
};

}
}

#endif
