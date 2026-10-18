// core/jet/include/res/resource.h
#ifndef CORE_JET_INCLUDE_RES_RESOURCE_H
#define CORE_JET_INCLUDE_RES_RESOURCE_H

#include <boost/noncopyable.hpp>
#include "core/pstring.h"
#include "stream/istream.h"

namespace jet::res {
class Resource : private boost::noncopyable {
protected:
    Resource(const Resource&);
public:
    Resource();
    virtual ~Resource();
    virtual bool Init(jet::stream::IStream*) = 0;
    virtual bool Load() = 0;
    virtual void Unload() = 0;
    virtual bool IsChanged();
    bool IsLoaded();
    void SetLoaded(bool);
    jet::stream::IStream *GetStream();
    bool HasStream();
    const jet::String &GetResourceName();
    void SetResourceName(const jet::String&);
    void FreeStreamWhenLoaded(bool);
protected:
    void SetStream(jet::stream::IStream*);
    void SwapContent(Resource*);
    Resource &operator=(const Resource&);
private:
    jet::stream::IStream *m_stream;
    jet::String m_resourceName;
    bool m_isLoaded;
    bool m_freeStreamWhenLoaded;
};
}

#endif
