// source/game/entities/ChunkTemplate.cpp
#include "ChunkTemplate.h"
#include "Deco3d.h"
#include "Object.h"
#include "PlatformInstance.h"

jet::String ChunkTemplate::k_tmplName("ChunkTemplate");

// @0x4d3754: the lanes in the order right, center, left
void ChunkTemplate::AddObjectsToSequence(PlatformInstance *p, unsigned offset) const {
    std::vector<const clara::Param*> params;
    params.push_back(FindParamByName(jet::String("Dummy_R")));
    params.push_back(FindParamByName(jet::String("Dummy_C")));
    params.push_back(FindParamByName(jet::String("Dummy_L")));
    p->AddSequencesFromParamConfig(params, p->GetCreatedEntity()->GetDeco()->GetModel(), offset);
}
