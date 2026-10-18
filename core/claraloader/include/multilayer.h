// core/claraloader/include/multilayer.h
#ifndef CORE_CLARALOADER_INCLUDE_MULTILAYER_H
#define CORE_CLARALOADER_INCLUDE_MULTILAYER_H

#include "clara.h"

namespace clara {

// Layer x entry table: each layer is a node path with one u32 per entry path.
class MultiLayer {
public:
    struct Layer {
        Path path;
        std::vector<unsigned> values;
    };

    MultiLayer() {}
    MultiLayer(const MultiLayer&) = delete;
    MultiLayer& operator=(const MultiLayer&) = delete;
    MultiLayer(MultiLayer&&) = default;

    void Load(jet::stream::IStream& s);
    void Unload() { m_layers.clear(); m_entries.clear(); }
    const jet::String& GetName() const { return m_name; }
    unsigned GetLayerCount() const { return unsigned(m_layers.size()); }
    const Layer& GetLayer(unsigned i) const { return m_layers[i]; }
    unsigned GetEntryCount() const { return unsigned(m_entries.size()); }
    const Path& GetEntry(unsigned i) const { return m_entries[i]; }

private:
    jet::String m_name;
    std::vector<Layer> m_layers;
    std::vector<Path> m_entries;
};

}

#endif
