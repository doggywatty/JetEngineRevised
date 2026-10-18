// source/game/entities/camera/CameraCinematicEntity.cpp
#include "CameraCinematicEntity.h"
#include "game/entities/Deco3d.h"
#include "param.h"
#include "project.h"

jet::String CameraCinematicEntity::k_tmplName("CameraCinematic");

// @0x4bb890: a library entity is left alone
void CameraCinematicEntity::Init() {
    if (IsLibraryEntity()) return;
    clara::Entity::Init();
}

// @0x4bb894
void CameraCinematicEntity::PostInit() {
    GameEntity::PostInit();
    if (!HasParam(jet::String("CinematicDeco"))) return;
    clara::Path path;
    if (const clara::Param *p = FindParamByName(jet::String("CinematicDeco"))) path = p->GetAsPath(0);
    if (clara::DataEntity *e = clara::Project::GetInstance()->FindEntityByPath(path))
        if (e->GetTemplateName().Equals(Deco3d::k_tmplName)) SetDeco(static_cast<Deco3d*>(e));
}

// @0x4bb9b0
void CameraCinematicEntity::SetDeco(Deco3d *deco) {
    m_deco = deco;
    jet::scene::Model *model = deco->GetModel();
    model->SetUpdateMode(0);
    delete m_cinematicCamera;
    m_cinematicCamera = new CinematicCamera;
    m_cinematicCamera->SetAnimatedCamera(model);
    m_cinematicCamera->SetLocalTransform(model->GetRootNode().GetPosition(), model->GetRootNode().GetRotation());
}
