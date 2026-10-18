// source/game/entities/BackgroundPool.cpp
#include "BackgroundPool.h"
#include "param.h"

const jet::String BackgroundPool::k_tmplName("BackgroundPool"), BackgroundPool::k_environmentTypeParam("environmentType"),
    BackgroundPool::k_backgroundDataPoolParam("backgroundDataPool");

// @0x5379fc
void BackgroundPool::PostInit() {
    clara::DataEntity::PostInit();
    GetParam(k_environmentTypeParam, m_environmentType);
    const clara::Param *p = FindParamByName(k_backgroundDataPoolParam);
    for (unsigned i = 0; i < p->GetComponentCount(); i++) m_backgrounds.push_back(p->GetAsPath(i));
}
