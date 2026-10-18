// core/jet/src/res/resource.cpp
#include "res/resource.h"

namespace jet::res {

Resource::Resource() : m_stream(0), m_isLoaded(false), m_freeStreamWhenLoaded(false) {}
Resource::Resource(const Resource&) : m_stream(0), m_isLoaded(false), m_freeStreamWhenLoaded(false) {}
Resource::~Resource() {}

bool Resource::IsChanged() { return m_stream && m_stream->IsChanged(); }
bool Resource::IsLoaded() { return m_isLoaded; }
void Resource::SetLoaded(bool v) { m_isLoaded = v; }

jet::stream::IStream *Resource::GetStream() { return m_stream; }
bool Resource::HasStream() { return m_stream != 0; }

const jet::String &Resource::GetResourceName() { return m_resourceName; }
void Resource::SetResourceName(const jet::String &n) { m_resourceName = n; }
void Resource::FreeStreamWhenLoaded(bool v) { m_freeStreamWhenLoaded = v; }

void Resource::SetStream(jet::stream::IStream *s) { m_stream = s; }

void Resource::SwapContent(Resource *o) {
    std::swap(m_stream, o->m_stream);
    std::swap(m_resourceName, o->m_resourceName);
    std::swap(m_isLoaded, o->m_isLoaded);
    std::swap(m_freeStreamWhenLoaded, o->m_freeStreamWhenLoaded);
}

Resource &Resource::operator=(const Resource &o) {
    m_resourceName = o.m_resourceName;
    return *this;
}

}
