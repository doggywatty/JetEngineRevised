// core/claraloader/include/template.h
#ifndef CORE_CLARALOADER_INCLUDE_TEMPLATE_H
#define CORE_CLARALOADER_INCLUDE_TEMPLATE_H

#include "type.h"

namespace clara {

// An entity class from a library: the ordered list of its parameters.
class Template {
public:
    struct Param {
        jet::String name;
        const Type* type = nullptr;
    };

    Template();
    Template(const Template&) = delete;
    Template& operator=(const Template&) = delete;

    void Load(jet::stream::IStream& s);
    const jet::String& GetName() const { return m_name; }
    unsigned GetId() const { return m_id; }
    unsigned GetParamCount() const { return unsigned(m_params.size()); }
    const Param& GetParam(unsigned i) const { return m_params[i]; }
    int FindParam(const jet::String& name) const;
    bool IsInternal() const { return m_isInternal; }

private:
    jet::String m_name;
    unsigned m_id;
    std::vector<Param> m_params;
    bool m_isInternal;
};

}

#endif
