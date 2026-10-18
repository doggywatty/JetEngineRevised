// core/jet/include/stream/filesystemdirstreamfactory.h
#ifndef CORE_JET_INCLUDE_STREAM_FILESYSTEMDIRSTREAMFACTORY_H
#define CORE_JET_INCLUDE_STREAM_FILESYSTEMDIRSTREAMFACTORY_H

#include <memory>
#include <unordered_set>
#include "stream/ifilesystem.h"
#include "stream/istreamfactory.h"
#include "thread/mutex.h"

namespace jet {
namespace stream {

// The files directly inside one directory of an IFileSystem.
class FileSystemDirStreamFactory : public IStreamFactory {
public:
    FileSystemDirStreamFactory(std::shared_ptr<IFileSystem> fs, const String& path, const String& ext);

    virtual IStream* CreateStream(const String& name);
    virtual void RefreshStreams();
    virtual std::vector<String> GetFileList(const String& ext) const;
    virtual unsigned GetFileCount() const;
    virtual String GetPath() const { return m_path; }
    virtual String GetExtension() const { return m_extension; }
    virtual bool CanCreateStream(const String& name) const;

private:
    String m_path;
    IFileSystem::Entry m_parent;
    std::shared_ptr<IFileSystem> m_filesystem;
    String m_extension;
    std::unordered_set<String> m_filenameMap;
    mutable thread::RecursiveMutex m_filenameMapMutex;
};

}
}

#endif
