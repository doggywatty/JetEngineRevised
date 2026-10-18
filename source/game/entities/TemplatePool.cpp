// source/game/entities/TemplatePool.cpp
#include "TemplatePool.h"
#include "param.h"

const jet::String TemplatePool::k_tmplName("TemplatePool"), TemplatePool::k_environmentTypeParam("environmentType"),
    TemplatePool::k_templateIdPoolParam("templateIdPool");

// @0x4b8f90
void TemplatePool::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(k_environmentTypeParam, m_environmentType);
    const clara::Param *p = FindParamByName(k_templateIdPoolParam);
    for (unsigned i = 0; i < p->GetComponentCount(); i++) m_templateIds.push_back((int)p->GetAsU32(i));
}
