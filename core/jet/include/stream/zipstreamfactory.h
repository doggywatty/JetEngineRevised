// core/jet/include/stream/zipstreamfactory.h
#ifndef CORE_JET_INCLUDE_STREAM_ZIPSTREAMFACTORY_H
#define CORE_JET_INCLUDE_STREAM_ZIPSTREAMFACTORY_H

#include <memory>
#include <unordered_set>
#include "stream/istreamfactory.h"
#include "thread/mutex.h"

namespace jet {
namespace stream {

class ZipFileSystem;

// The files at the root of a zip (or .jpk) archive.
class ZipStreamFactory : public IStreamFactory {
public:
    ZipStreamFactory(const String& path, const String& ext);

    virtual IStream* CreateStream(const String& name);
    virtual void RefreshStreams();
    virtual std::vector<String> GetFileList(const String& ext) const;
    virtual unsigned GetFileCount() const;
    virtual String GetPath() const { return m_path; }
    virtual String GetExtension() const { return m_extension; }
    virtual bool CanCreateStream(const String& name) const;

private:
    String m_path;
    String m_extension;
    std::shared_ptr<ZipFileSystem> m_zfs;
    std::unordered_set<String> m_filenameMap;
    mutable thread::RecursiveMutex m_filenameMapMutex;
};

}
}

#endif
