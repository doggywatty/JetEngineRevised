// core/jet/include/stream/dirstreamfactory.h
#ifndef CORE_JET_INCLUDE_STREAM_DIRSTREAMFACTORY_H
#define CORE_JET_INCLUDE_STREAM_DIRSTREAMFACTORY_H

#include <unordered_set>
#include "stream/istreamfactory.h"
#include "thread/mutex.h"

namespace jet {
namespace stream {

// The files directly inside a host directory.
class DirStreamFactory : public IStreamFactory {
public:
    DirStreamFactory(const String& path, const String& ext);

    virtual IStream* CreateStream(const String& name);
    virtual void RefreshStreams();
    virtual std::vector<String> GetFileList(const String& ext) const;
    virtual unsigned GetFileCount() const;
    virtual String GetPath() const { return m_path; }
    virtual String GetExtension() const { return m_extension; }
    virtual bool CanCreateStream(const String& name) const;

private:
    String m_path;
    String m_path2;  // m_path with a trailing '/'
    String m_extension;
    std::unordered_set<String> m_filenameMap;
    mutable thread::RecursiveMutex m_filenameMapMutex;
};

}
}

#endif
