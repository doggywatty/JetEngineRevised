// core/jet/include/stream/istreamfactory.h
#ifndef CORE_JET_INCLUDE_STREAM_ISTREAMFACTORY_H
#define CORE_JET_INCLUDE_STREAM_ISTREAMFACTORY_H

#include <vector>
#include "core/pstring.h"

namespace jet {
namespace stream {

class IStream;

// A source of named streams (a folder, an archive...). Names are case-insensitive.
class IStreamFactory {
public:
    IStreamFactory() {}
    IStreamFactory(const IStreamFactory&) = delete;
    IStreamFactory& operator=(const IStreamFactory&) = delete;
    virtual ~IStreamFactory() {}

    virtual IStream* CreateStream(const String& name) = 0;
    virtual void RefreshStreams() = 0;
    virtual std::vector<String> GetFileList(const String& ext) const = 0;
    virtual unsigned GetFileCount() const = 0;
    virtual String GetPath() const = 0;
    virtual String GetExtension() const = 0;
    virtual bool CanCreateStream(const String& name) const = 0;
};

// Files in the set whose extension is ext (all when ext is empty or "*").
template <class Set>
std::vector<String> FilterByExtension(const Set& names, const String& ext)
{
    std::vector<String> r;
    bool all = ext.empty() || ext == "*";
    for (const String& n : names) {
        if (all) {
            r.push_back(n);
            continue;
        }
        unsigned dot = n.find_last_of('.');
        if (dot != String::npos && n.size() - dot - 1 == ext.size() && n.find(ext.c_str(), dot + 1) == dot + 1)
            r.push_back(n);
    }
    return r;
}

}
}

#endif
