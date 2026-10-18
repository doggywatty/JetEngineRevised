// source/game/entities/PlatformInstance.cpp
#include "PlatformInstance.h"
#include <algorithm>
#include "ChunkTemplate.h"
#include "Deco3d.h"
#include "GameEntityCreator.h"
#include "InteractiveObjectTemplate.h"
#include "Object.h"
#include "ObstacleTemplate.h"
#include "Runner.h"
#include "SafeMinionPoint.h"
#include "Launcher.h"
#include "PlatformTemplate.h"
#include "../Game.h"
#include "../GameLevel.h"
#include "../Player.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../leveltemplate/LevelTemplateDef.h"
#include "../utils/ControlledRandom.h"
#include "../utils/GameUtils.h"
#include "param.h"
#include "project.h"
#include "scene/model.h"
#include "scene/modelbase.h"
#include "scene/modelloader.h"
#include "scene/scenemgr.h"

jet::String PlatformInstance::k_tmplName("PlatformInstance");
const jet::String PlatformInstance::k_DummyHelperInstance_tmplName("DummyHelperInstance"), PlatformInstance::k_DummyHelper_tmplName("DummyHelper"),
    PlatformInstance::k_ChunkTemplateSelector_tmplName("ChunkTemplateSelector");

// the lane of a sequence (right, center, left) as the entities' spawn lanes (left 0, center 1, right 2)
static const int k_spawnLane[3] = { 2, 1, 0 };

// @0x4f79f0
PlatformInstance::PlatformInstance(const clara::Template *t)
    : GameEntity(t), m_createdObject(0), m_keepCreatedEntity(false), m_platformTemplate(0), m_useRandomizer(false), m_randomizerSizeMult(1.0f), m_numDummies(1), m_numChunks(0),
      m_clonedFromRandomIndex(0), m_nextCloned(0), m_baseInstance(0), m_hasSingleLane(false), m_singleLane(0), m_addDefaultSafeMinionPoint(false),
      m_gameplayTemplateInstance(0) {
    m_type = 0xc;
}

// @0x4f7a70
PlatformInstance::~PlatformInstance() {
    if (!m_keepCreatedEntity) {
        delete m_createdObject;
        for (GameEntity *e : m_createdLinkedObjects) delete e;
        m_createdLinkedObjects.clear();
    }
    for (PlatformInstance *c : m_clonedList) delete c;
}

const math::vec3<float> &PlatformInstance::GetPosition() const { return m_createdObject ? m_createdObject->GetPosition() : GameEntity::GetPosition(); }
void PlatformInstance::SetPosition(const math::vec3<float> &p) {
    if (m_createdObject) m_createdObject->SetPosition(p);
    clara::Entity::SetPosition(p);
}
const math::quat<float> &PlatformInstance::GetRotation() const { return m_createdObject ? m_createdObject->GetRotation() : GameEntity::GetRotation(); }
void PlatformInstance::SetRotation(const math::quat<float> &r) {
    if (m_createdObject) m_createdObject->SetRotation(r);
    clara::Entity::SetRotation(r);
}
void PlatformInstance::Translate(const math::vec3<float> &d) {
    if (m_createdObject) m_createdObject->Translate(d);
}

// @0x4d3658
void PlatformInstance::SetPersistenceType(int t) {
    GameEntity::SetPersistenceType(t);
    if (m_createdObject) m_createdObject->SetPersistenceType(t);
    for (GameEntity *e : m_createdLinkedObjects) e->SetPersistenceType(t);
}

// @0x4f9a24
void PlatformInstance::OnPositionedInWorld() {
    if (m_createdObject) m_createdObject->OnPositionedInWorld();
}

// @0x4f7bb0
void PlatformInstance::ConstructFrom(const GameEntity *o) {
    const PlatformInstance *p = static_cast<const PlatformInstance*>(o);
    m_clonedFromRandomIndex = p->m_clonedFromRandomIndex;
    GameEntity::ConstructFrom(o);
    m_randomizerSizeMult = p->m_randomizerSizeMult;
    m_numDummies = p->m_numDummies;
    m_numChunks = p->m_numChunks;
    m_useRandomizer = p->m_useRandomizer;
    m_nextCloned = p->m_nextCloned;
    m_baseInstance = p->m_baseInstance;
}

// @0x4f7bfc: the PlatformTemplate it follows is the one its Template parameter lists at the index of its copy
void PlatformInstance::Init() {
    const clara::Param *p = FindParamByName(jet::String("Template"));
    m_platformTemplate = static_cast<PlatformTemplate*>(
        clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath((unsigned)m_clonedFromRandomIndex % p->GetComponentCount())));
    if (m_flags & F_LIBRARY) {
        InitRandomizer();
        return;
    }
    GameEntity::Init();
}

// @0x4f7c7c: a copy of the template takes the platform's place, and its object is kept
void PlatformInstance::PostInit() {
    if (m_flags & F_LIBRARY) return;
    PlatformTemplate *tmpl = new PlatformTemplate(clara::Project::GetInstance()->FindTemplateByName(PlatformTemplate::k_tmplName));
    PlatformTemplate *ref = m_platformTemplate;
    const math::vec3<float> pos = ref->GetPosition(), scale = ref->GetScale();
    const math::quat<float> rot = ref->GetRotation();
    ref->SetPosition(pos + GetPosition());
    ref->SetRotation(rot * GetRotation());
    const math::vec3<float> mine = GetScale();
    ref->SetScale(math::vec3<float>(mine.x * scale.x, mine.y * scale.y, mine.z * scale.z));
    tmpl->ConstructFrom(static_cast<const GameEntity*>(ref));
    ref->SetPosition(pos);
    ref->SetRotation(rot);
    ref->SetScale(scale);
    m_createdObject = tmpl->GetCreatedEntity();
    tmpl->SetKeepCreatedEntity(true);
    delete tmpl;
    GetParam(jet::String("hasSingleLane"), m_hasSingleLane);
    if (m_hasSingleLane) GetParam(jet::String("SingleLane"), reinterpret_cast<int&>(m_singleLane));
    GetParam(jet::String("addDefaultSafeMinionPoint"), m_addDefaultSafeMinionPoint);
    RegisterForUpdate(true);
    RegisterForRender(false);
}

// @0x4f8258
unsigned PlatformInstance::GetChunkCount(clara::DataEntity *chunks) const {
    int interval = 0, num = 0;
    chunks->GetParam(jet::String("ChunkInterval"), interval);
    chunks->GetParam(jet::String("numchunks"), num);
    return (unsigned)(num * interval);
}

// @0x4f7f0c: how many dummies a lane of the model has and how long the randomized platform is
void PlatformInstance::InitRandomizer() {
    GetParam(jet::String("useRandomizer"), m_useRandomizer);
    m_numChunks = 0;
    m_randomizerSizeMult = 1.0f;
    m_numDummies = 1;
    jet::String modelName;
    if (GameEntity *deco = m_platformTemplate->GetDecoReference()) deco->GetParam(Deco3d::k_model, modelName);
    boost::shared_ptr<jet::scene::ModelBase> model;
    if (modelName.size()) {
        jet::scene::ModelLoader::Params params;
        params.loadTextures = false;
        params.loadRenderTechniques = false;
        model = jet::scene::ModelLoader::GetInstance()->LoadModelBase(modelName, params);
    }
    if (!model) return;
    static const char *const k_dummies[3] = { "Dummy_R", "Dummy_C", "Dummy_L" };
    for (const char *d : k_dummies) {
        const jet::String name = FindParamByName(jet::String(d))->GetName();
        unsigned count = 0;
        for (unsigned i = 0; i < model->GetNodeCount(); i++) {
            const jet::scene::ModelBase::NodeData &n = model->GetNode(i);
            if ((n.flags & jet::scene::ModelBase::NodeFlags::Dummy) && n.name.find(name.c_str(), 0) != (size_t)-1) count++;
        }
        if (count > m_numDummies) m_numDummies = count;
    }
    Game::GetInstance()->AddRandomizerModel(model);
    if (m_useRandomizer) {
        if (const clara::Param *chunks = FindParamByName(jet::String("RandomizeChunks"))) {
            for (unsigned i = 0; i < chunks->GetComponentCount(); i++) m_numChunks += GetChunkCount(chunks->GetAsEntity(i));
            m_randomizerSizeMult = (float)(m_numChunks / m_numDummies) + (m_numChunks % m_numDummies == 0 ? 0.0f : 1.0f);
        }
    }
}

// @0x4f97fc: a platform of several lengths is made of copies one length apart; they follow each other
void PlatformInstance::ProcessRandomizerData(LevelTemplateDef *def) {
    m_clonedList.clear();
    if (!(m_randomizerSizeMult > 1.0f)) return;
    const float length = def->GetSizeY();
    def->SetScaleMult(m_randomizerSizeMult);
    m_baseInstance = this;
    for (unsigned i = 1; i < (unsigned)(m_randomizerSizeMult > 0 ? (int)m_randomizerSizeMult : 0); i++) {
        GameEntityCreator creator;
        PlatformInstance *c = static_cast<PlatformInstance*>(creator.CreateEntity(GetTemplate()));
        c->SetFlag(F_LIBRARY, (m_flags & F_LIBRARY) != 0);
        m_clonedFromRandomIndex = (int)i;
        c->ConstructFrom(static_cast<const GameEntity*>(this));
        const math::vec3<float> &right = jet::scene::SceneMgr::GetRightVector();
        const math::vec3<float> step = right * (length * (float)i);
        c->SetPosition(c->GetPosition() - math::transform(c->GetRotation().getAsMat3(), step));
        m_clonedList.push_back(c);
        def->AddEntity(c);
    }
    m_clonedFromRandomIndex = 0;
    if (m_clonedList.empty()) return;
    m_nextCloned = m_clonedList[0];
    for (size_t k = 0; k + 1 < m_clonedList.size(); k++) m_clonedList[k]->m_nextCloned = m_clonedList[k + 1];
    m_clonedList.back()->m_nextCloned = 0;
}

// @0x4f82c4: the chunks of a randomized platform, from its first copy
void PlatformInstance::AddSequencesFromRandomizer() {
    const clara::Param *chunks = FindParamByName(jet::String("RandomizeChunks"));
    unsigned offset = 0;
    if (m_clonedFromRandomIndex == 0 && chunks)
        for (unsigned i = 0; i < chunks->GetComponentCount(); i++) AddSequencesFromChunk(chunks->GetAsEntity(i), offset);
}

// @0x4f8348: the chunks drawn among a group's list, by weights, each of numchunks following the one before by ChunkInterval
void PlatformInstance::AddSequencesFromChunk(clara::DataEntity *group, unsigned &offset) {
    bool forced = false;
    if (!Player::GetInstance()->IsSideLookTutorialDone()) {
        const jet::String name = group->GetClaraFile()->GetName();
        forced = name.Equals("045_LAB") || name.Equals("344_LAB") || name.Equals("345_LAB");
    }
    int interval = 0, num = 0;
    group->GetParam(jet::String("ChunkInterval"), interval);
    group->GetParam(jet::String("numchunks"), num);
    const clara::Param *list = group->FindParamByName(jet::String("chunks"));
    std::vector<ChunkTemplate*> pool;
    std::vector<float> weights;
    const float w = 100.0f / (float)list->GetComponentCount();
    for (unsigned i = 0; i < list->GetComponentCount(); i++) {
        ChunkTemplate *c = static_cast<ChunkTemplate*>(clara::Project::GetInstance()->FindEntityByPath(list->GetAsPath(i)));
        if (!c) continue;
        size_t k = std::find(pool.begin(), pool.end(), c) - pool.begin();
        if (k < pool.size()) weights[k] += w;
        else {
            weights.push_back(w);
            pool.push_back(c);
        }
    }
    if (pool.empty()) return;
    const std::vector<unsigned> picks = GameUtils::IndexDistributedRandom(weights, (unsigned)num, GameLevel::s_randomizer[1]);
    for (unsigned k = 0; k < (unsigned)num; k++) {
        pool[forced ? 0 : picks[k]]->AddObjectsToSequence(this, offset);
        forced = false;
        offset += (unsigned)interval;
    }
}

// @0x4f91e4: the entities a dummy parameter lists, each at the dummy index its config gives (plus the chunk's offset)
void PlatformInstance::AddToSequence(clara::DataEntity *config, const jet::String &dummyName, unsigned offset, unsigned lane, std::vector<GenerationSequence> &out) {
    unsigned index = 0;
    if (config->FindParamByName(jet::String("index"))) {
        config->GetParam(jet::String("index"), reinterpret_cast<int&>(index));
        index += offset;
    }
    const clara::Param *list = config->FindParamByName(jet::String("Template"));
    for (unsigned i = 0; i < list->GetComponentCount(); i++) {
        const clara::Path &path = list->GetAsPath(i);
        if (path.IsEmpty()) continue;
        GameEntity *e = dynamic_cast<GameEntity*>(clara::Project::GetInstance()->FindEntityByPath(path));
        if (!e) continue;
        GenerationSequence s = { e, dummyName, index, lane, m_gameplayTemplateInstance, 0, 0 };
        out.push_back(s);
    }
}

// @0x4f8700
void PlatformInstance::AddSequencesFromParamConfig(const std::vector<const clara::Param*> &params, jet::scene::Model *model, unsigned offset) {
    if (params.empty() || !model->GetBase()->GetDummyCount()) return;
    PlatformInstance *target = m_baseInstance ? m_baseInstance : this;
    for (unsigned lane = 0; lane < params.size(); lane++) {
        const clara::Param *p = params[lane];
        const jet::String &name = p->GetName();
        if (!model->GetDummyCountByName(name)) continue;
        for (unsigned i = 0; i < p->GetComponentCount(); i++) AddToSequence(p->GetAsEntity(i), name, offset, lane, target->m_generationSequence);
    }
}

// @0x4f87e8
void PlatformInstance::AddSequencesFromParamConfig() {
    std::vector<const clara::Param*> params;
    params.push_back(FindParamByName(jet::String("Dummy_R")));
    params.push_back(FindParamByName(jet::String("Dummy_C")));
    params.push_back(FindParamByName(jet::String("Dummy_L")));
    AddSequencesFromParamConfig(params, m_createdObject->GetDeco()->GetModel(), 0);
}

// @0x4f8ab8: a copy of a randomized platform takes the objects of its length from the first one's sequence
void PlatformInstance::CopySequenceToCreate() {
    if (m_baseInstance) {
        std::vector<GenerationSequence> &base = m_baseInstance->m_generationSequence;
        std::vector<size_t> taken;
        for (size_t i = 0; i < base.size(); i++)
            if (base[i].gti == m_gameplayTemplateInstance && (int)(base[i].index / m_numDummies) == m_clonedFromRandomIndex) {
                m_generationSequence.push_back(base[i]);
                ProcessCopiedSequence(m_generationSequence.size() - 1, m_generationSequence);
                taken.push_back(i);
            }
        for (size_t k = taken.size(); k > 0; k--) base.erase(base.begin() + taken[k - 1]);
    }
    m_gameplayTemplateInstance->BuildAllowedDecoList(0);
}

// @0x4f9454: the object is made from the entity the sequence names, linked to the dummy of the platform's deco
GameEntity *PlatformInstance::CreateLinkedObject(GameEntity *tmpl, jet::scene::Dummy *dummy, unsigned lane, unsigned index, GameplayTemplateInstance *gti) {
    if (!dummy || !tmpl) return 0;
    GameEntityCreator creator;
    GameEntity *obj = dynamic_cast<GameEntity*>(creator.CreateEntity(tmpl->GetTemplate()));
    obj->SetSpawnLane(k_spawnLane[lane]);
    obj->SetDistanceInRun(gti->GetStartDistance() + (gti->GetDefinition()->GetSizeY() / (float)(gti->GetNumTotalDummies() - 1)) * (float)index);
    obj->ConstructFrom(static_cast<const GameEntity*>(tmpl));
    obj->SetPersistenceType(GameEntity::PersistenceTypeDef::Volatile);
    m_createdLinkedObjects.push_back(obj);
    const jet::String &linkPoint = dummy->GetNode()->GetName();
    Deco3d *deco = m_createdObject->GetDeco();
    if (!obj->GetLinkee()) {
        obj->SetPosition(math::vec3<float>(0, 0, 0));
        obj->SetRotation(math::quat<float>(0, 0, 0, 1));
        obj->LinkTo(deco, linkPoint, 0);
        obj->UpdateTransform(false);
        obj->SetPosition(tmpl->GetPosition());
        obj->SetRotation(tmpl->GetRotation());
    } else {
        clara::Entity *l = obj->GetLinkee();
        l->SetPosition(math::vec3<float>(0, 0, 0));
        l->SetRotation(math::quat<float>(0, 0, 0, 1));
        l->LinkTo(deco, linkPoint, 0);
    }
    // the objects that need the platform to be made ready: an obstacle blocks the lanes it covers along the template
    if (ObstacleTemplate *o = dynamic_cast<ObstacleTemplate*>(obj)) {
        o->SetLane((LaneIndex)k_spawnLane[lane]);
        o->InitCollisionLanes();
        o->SetCollisionStart(index);
        o->SetGameplayTemplateInstance(gti);
        o->EnableTileCollisions(true);
    }
    // the launcher makes its pushers, placed at its place
    if (Launcher *l = dynamic_cast<Launcher*>(obj)) l->InitLauncherData();
    if (SafeMinionPoint *s = dynamic_cast<SafeMinionPoint*>(obj)) {
        s->Register(m_gameplayTemplateInstance);
        s->ComputeCollisionPlane();
    }
    obj->OnPositionedInWorld();
    return obj;
}

// @0x519ef4: a copied sequence element, in the list of the platform it is copied to. An obstacle brings its linked interactive template along, as
// a copy at the same dummy (processed in turn). A runner, and an interactive object named superfluous, join the entity generation group of the
// template at their index; the group keeps whether the object is forced to be made.
void PlatformInstance::ProcessCopiedSequence(size_t i, std::vector<GenerationSequence> &list) {
    GameEntity *entity = list[i].entity;
    if (dynamic_cast<ObstacleTemplate*>(entity)) {
        GameEntity *linked = static_cast<ObstacleTemplate*>(entity)->GetLinkedInteractiveTemplate();
        if (!linked) return;
        const GenerationSequence c = { linked, list[i].dummyName, list[i].index, list[i].lane, list[i].gti, 0, 0 };
        list.push_back(c);
        ProcessCopiedSequence(list.size() - 1, list);
        return;
    }
    InteractiveObjectTemplate *interactive = dynamic_cast<InteractiveObjectTemplate*>(entity);
    if (!dynamic_cast<Runner*>(entity) && !(interactive && entity->GetTemplateName().Equals(InteractiveObjectTemplate::k_superflousInteractive_tmplName)))
        return;
    GenerationSequence &s = list[i];
    if (!s.group) s.group = new GenerationGroupEntity();
    m_gameplayTemplateInstance->AddToEntityGenerationGroup(s.group, s.index);
    if (interactive) s.group->forced = interactive->GetForzeSpawnIt();
}

// @0x51a41c: an object made from a sequence element. An obstacle covers the elements of its lane from its index over its collision tiles: they keep
// it as their created object, the interactive objects (and every element when the obstacle has no pacesetter of its own) or the others than the
// runners (when it has one). An element that an obstacle covers gives an interactive object to the obstacle; any other object that is not an
// obstacle is placed in the frame of the obstacle's deco (linked to it, at the offset from the obstacle, turned by its rotation).
void PlatformInstance::ProcessGeneratedLinkedObject(GameEntity *obj, size_t i, GameplayTemplateInstance *gti, std::vector<GenerationSequence> &list) {
    if (!gti || !obj) return;
    if (ObstacleTemplate *o = dynamic_cast<ObstacleTemplate*>(obj)) {
        const unsigned first = list[i].index, end = first + o->GetCollisionTiles();
        const bool ownPacesetter = o->GetOwnPacesetter() != 0;
        for (GenerationSequence &e : list) {
            if (e.lane != list[i].lane || e.index < first || e.index >= end) continue;
            if (ownPacesetter ? dynamic_cast<Runner*>(e.entity) != 0 : dynamic_cast<InteractiveObjectTemplate*>(e.entity) == 0) continue;
            e.created = obj;
        }
    }
    ObstacleTemplate *cover = dynamic_cast<ObstacleTemplate*>(list[i].created);
    if (!cover) return;
    if (dynamic_cast<InteractiveObjectTemplate*>(obj)) {
        cover->SetLinkedInteractiveInstance(obj);
        return;
    }
    if (dynamic_cast<ObstacleTemplate*>(obj)) return;
    const math::vec3<float> posObj = obj->GetPosition(), posCover = cover->GetPosition();
    const math::quat<float> rotObj = obj->GetRotation(), rotCover = cover->GetRotation();
    const math::vec3<float> offset = posObj - posCover;
    obj->LinkTo(0, jet::String::null, 0);
    obj->LinkTo(cover->GetDeco(), jet::String::null, 0);
    const math::quat<float> inv(-rotCover.x, -rotCover.y, -rotCover.z, rotCover.w);
    obj->SetPosition(inv * offset);
    obj->SetRotation(inv * rotObj);
}

// @0x4f8fd8
bool PlatformInstance::CreateObjectsFromSequence(GameplayTemplateInstance *gti, int count) {
    jet::scene::Model *model = m_createdObject->GetDeco()->GetModel();
    if (count < 1) count = (int)m_generationSequence.size();
    std::vector<size_t> done;
    int made = 0;
    for (size_t i = 0; made < count && i < m_generationSequence.size(); i++) {
        GenerationSequence &s = m_generationSequence[i];
        if ((int)(s.index / m_numDummies) != m_clonedFromRandomIndex) continue;
        if (s.entity->CanSpawnNewEntity(gti) && gti->IsEntityAllowed(s.group)) {
            jet::scene::Dummy *d = model->FindDummyByNameAndIndex(s.dummyName, s.index % m_numDummies);
            GameEntity *obj = CreateLinkedObject(s.entity, d, s.lane, s.index, gti);
            ProcessGeneratedLinkedObject(obj, i, gti, m_generationSequence);
        }
        done.push_back(i);
        made++;
    }
    for (size_t k = done.size(); k > 0; k--) {
        delete m_generationSequence[done[k - 1]].group;
        m_generationSequence.erase(m_generationSequence.begin() + done[k - 1]);
    }
    return m_generationSequence.empty();
}
