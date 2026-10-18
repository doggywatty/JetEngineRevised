// source/game/entities/TemplateData.cpp
#include "TemplateData.h"
#include "param.h"
#include "project.h"
#include "PlatformInstance.h"

const jet::String TemplateData::k_tmplName("TemplateData"), TemplateData::k_groundPlatform("groundPlatform"),
    TemplateData::k_IsShootingSection("IsShootingSection"), TemplateData::k_isBonusSection("IsBonusSection"),
    TemplateData::k_isLaunchSection("IsLaunchSesction"), TemplateData::k_isGameplayTemplate("IsGameplayTemplate"),
    TemplateData::k_isTransitionSection("IsTransitionSection");

// @0x4b8d50
void TemplateData::Init() {
    clara::Entity::Init();
    GetParam(k_IsShootingSection, m_isShootingSection);
    if (HasParam(k_isBonusSection)) GetParam(k_isBonusSection, m_isBonusSection);
    if (HasParam(k_isLaunchSection)) GetParam(k_isLaunchSection, m_isLaunchSection);
    if (HasParam(k_isGameplayTemplate)) GetParam(k_isGameplayTemplate, m_isGameplayTemplate);
    if (HasParam(k_isTransitionSection)) GetParam(k_isTransitionSection, m_isTransitionSection);
}

// @0x4b8e68
PlatformInstance *TemplateData::GetGroundPlatform(bool) {
    if (!m_groundPlatform)
        m_groundPlatform = static_cast<PlatformInstance*>(clara::Project::GetInstance()->FindEntityByPath(FindParamByName(k_groundPlatform)->GetAsPath(0)));
    return m_groundPlatform;
}
