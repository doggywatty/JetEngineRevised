// source/game/leveltemplate/LevelSequenceGraph.cpp
#include "game/leveltemplate/LevelSequenceGraph.h"
#include <algorithm>
#include <cstdlib>
#include "game/GameLevel.h"
#include "game/colorprofiles.h"
#include "game/gamemanagers/EdgeMgr.h"
#include "game/gamemanagers/EdgeProbabilityMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/leveltemplate/LevelSequenceGraphParser.h"
#include "game/leveltemplate/LevelTemplateMgr.h"
#include "game/utils/ControlledRandom.h"
#include "game/utils/GameUtils.h"

const jet::String LevelSequenceGraph::k_initialEntryPoint("0"), LevelSequenceGraph::k_ProbabilityEvolutionSet_tmplName("ProbabilityEvolutionSet"),
    LevelSequenceGraph::k_ProbabilityIntervalDef_tmplName("ProbabilityIntervalDef"), LevelSequenceGraph::k_leftStr("left"), LevelSequenceGraph::k_rightStr("right"),
    LevelSequenceGraph::k_type1Str("type1"), LevelSequenceGraph::k_type2Str("type2"), LevelSequenceGraph::k_defaultStr("default");
std::map<jet::String, float> LevelSequenceGraph::s_probabilityMap;
float LevelSequenceGraph::s_totalProbabilityWeight = 0;
int LevelSequenceGraph::s_specialExitChoosen[4] = { 0, 0, 0, 0 };

// @0x3e33a8: the edges, the nodes and what hangs on them
LevelSequenceGraph::~LevelSequenceGraph() {
    for (auto &kv : m_edgeMgrs) delete kv.second;
    for (auto &kv : m_nodes) {
        Node *n = kv.second;
        for (Edge *e : n->edges) delete e;
        for (Edge *e : n->managedEdges) delete e;
        delete n->sequence;
        delete n;
    }
    for (auto &kv : m_specialNodes) delete kv.second;
    for (ExitPoint *e : m_exitPointList) delete e;
}

// @0x3e36dc
void LevelSequenceGraph::AddNode(const char *id, const char *label, LevelSequence *s) { m_nodes[jet::String(id)] = new Node(id, label, s); }

// @0x3e37ec
void LevelSequenceGraph::AddTutorialNode(const char *id, const char *label, LevelSequence *s) {
    AddNode(id, label, s);
    m_nodes[jet::String(id)]->type = Node::Tutorial;
}

// @0x3e3840
void LevelSequenceGraph::AddExitPoint(const char *id, const char *label, const jet::String &toLabel, int location) {
    AddNode(id, label, 0);
    m_nodes[jet::String(id)]->type = Node::ExitPoint;
    ExitPoint *e = new ExitPoint;
    e->label = toLabel;
    e->location = location;
    m_exitPointList.push_back(e);
    m_exitPoints[jet::String(id)] = e;
}

// @0x3e3940
void LevelSequenceGraph::AddRandomHub(const char *id, const char *label) {
    AddNode(id, label, 0);
    m_nodes[jet::String(id)]->type = Node::RandomHub;
}

// @0x3e3998: a hub that finishes the first play tutorial once it is left
void LevelSequenceGraph::AddTutorialFinishedNode(const char *id, const char *label) {
    AddRandomHub(id, label);
    m_nodes[jet::String(id)]->onCompleted = [this]() { OnCompletedTutorial(); };
}

// @0x3e3a74
void LevelSequenceGraph::AddSpecialHub(const char *id, const char *label, const char *name) {
    AddNode(id, label, 0);
    Node *n = m_nodes[jet::String(id)];
    n->type = Node::SpecialHub;
    SpecialNode *s = new SpecialNode;
    s->node = n;
    s->unused = 0;
    m_specialNodes[jet::String(name)] = s;
}

// @0x3e3b3c
void LevelSequenceGraph::AddProbabilityNode(const char *id, const char *label, LevelSequence *s, int percent) {
    AddNode(id, label, s);
    Node *n = m_nodes[jet::String(id)];
    n->type = Node::Probability;
    n->probability = (unsigned char)percent;
}

// @0x3e3bc4
void LevelSequenceGraph::AddEntryPoint(const char *id, const char *label, const char *name) {
    m_locationEntryPoint[jet::String(name)] = jet::String(id);
    AddRandomHub(id, label);
}

// @0x3e3c24: both lists of the source node
void LevelSequenceGraph::AddEdge(const char*, const char *label, const char *key, const char *source, const char *target) {
    Edge *e = new Edge(label, key, source, target);
    Node *n = m_nodes.at(jet::String(source));
    n->edges.push_back(e);
    n->pool.push_back(e);
}

// @0x3e3d24
void LevelSequenceGraph::AddProbabilityEdge(const char*, const char *label, const char *key, const char *source, const char *target, LevelSequenceParser &parser) {
    Edge *e = new Edge(label, key, source, target);
    Node *n = m_nodes.at(jet::String(source));
    auto it = m_edgeMgrs.find(jet::String(key));
    if (it == m_edgeMgrs.end()) {
        EdgeMgr *m = new EdgeProbabilityMgr;
        m->Create(jet::String(key), parser);
        it = m_edgeMgrs.insert(std::make_pair(jet::String(key), m)).first;
    }
    e->mgr = it->second;
    n->managedEdges.push_back(e);
}

// @0x3e3ec0: an edge of a condition (the label of the edge says which)
void LevelSequenceGraph::AddConditionalEdge(const char*, const char *label, const char *key, const char *source, const char *target, LevelSequenceParser &parser) {
    Edge *e = new Edge(label, "", source, target);
    Node *n = m_nodes.at(jet::String(source));
    auto it = m_edgeMgrs.find(jet::String(key));
    if (it == m_edgeMgrs.end()) {
        EdgeMgr *m = new EdgeConditionMgr;
        m->Create(jet::String(key), parser);
        it = m_edgeMgrs.insert(std::make_pair(jet::String(key), m)).first;
    }
    e->mgr = it->second;
    n->managedEdges.push_back(e);
}

// @0x3e4060
void LevelSequenceGraph::SetLevelSequenceGraphEntryPoint(const jet::String &name) {
    const bool started = m_sequence != 0;
    m_currentNode = GetNextNode(m_locationEntryPoint[name], -1);
    m_sequence = m_currentNode->sequence;
    if (!started) InitLevelSequenceGraph();
}

// @0x3e40a4
void LevelSequenceGraph::InitLevelSequenceGraph() {
    for (auto &kv : m_specialNodes) MarkNodeAsInSpecialTree(kv.second->node);
}

// @0x3e41a0
void LevelSequenceGraph::MarkNodeAsInSpecialTree(Node *n) {
    if (!n || n->inSpecialTree) return;
    n->inSpecialTree = true;
    for (Edge *e : n->edges) MarkNodeAsInSpecialTree(m_nodes[e->target]);
}

// @0x3e4200
LevelSequenceGraph::Edge *LevelSequenceGraph::FindEdgeByLabel(const std::vector<Edge*> &v, const jet::String &label) const {
    for (Edge *e : v)
        if (e->label.Equals(label)) return e;
    return 0;
}

// @0x3e4c3c
const jet::String &LevelSequenceGraph::GetDesiredLabel(Bifurcation b) const { return b == Bifurcation_Right ? k_rightStr : k_leftStr; }

// @0x3e423c: from a node, the one the graph leads to (a tee takes the way the player chose)
LevelSequenceGraph::Node *LevelSequenceGraph::GetNextNode(const jet::String &id, int finishedTemplate) {
    bool useBifurcation = false;
    Bifurcation bif = Bifurcation_Center;
    if (finishedTemplate >= 0 && LevelTemplateMgr::GetBifurcation(finishedTemplate) == Bifurcation_Tee) {
        useBifurcation = true;
        bif = (Bifurcation)GameLevel::GetInstance()->GetFinalBifurcation();
    }
    Node *next;
    if (!m_isOnSpecialGraph || !m_nodes[id]->edges.empty()) {
        next = GetNextNodeByEdge(GetNextEdge(m_nodes[id], useBifurcation, bif));
    } else {
        next = m_specialReturn;
        m_isOnSpecialGraph = false;
        m_specialReturn = 0;
        m_specialEdgeInterval = -1;
    }
    while (!IsValidNode(next)) next = JumpToNextNode(next);
    return next;
}

// @0x3d4340
float LevelSequenceGraph::GetAverageProbabilityWeight() {
    return s_probabilityMap.empty() ? 10.0f : s_totalProbabilityWeight / (float)s_probabilityMap.size();
}

// @0x3e5058
void LevelSequenceGraph::SetProbabilityWeight(const jet::String &id, float w) {
    auto it = s_probabilityMap.find(id);
    if (it == s_probabilityMap.end()) {
        s_probabilityMap[id] = w;
        s_totalProbabilityWeight += w;
    } else {
        const float old = it->second;
        it->second = w;
        s_totalProbabilityWeight = (s_totalProbabilityWeight - old) + w;
    }
}

// @0x3e50e4
float LevelSequenceGraph::GetProbabilityWeight(const jet::String &id) {
    auto it = s_probabilityMap.find(id);
    return it == s_probabilityMap.end() ? -1.0f : it->second;
}

// @0x3e43c8
void LevelSequenceGraph::ResetStaticData() {
    s_probabilityMap.clear();
    s_totalProbabilityWeight = 0;
}

// @0x3e4420: the edge of the desired way (a tee), else one of those a manager allows picked by weights, else one at random of those not taken yet
LevelSequenceGraph::Edge *LevelSequenceGraph::GetNextEdge(Node *n, bool useBifurcation, Bifurcation bif) {
    if (useBifurcation) {
        const jet::String &want = GetDesiredLabel(bif);
        Edge *last = 0;
        for (Edge *e : n->edges) {
            last = e;
            if (e->label.Equals(want)) break;
        }
        if (last) return last;
    }
    if (!n->managedEdges.empty()) {
        std::vector<Edge*> candidates;
        std::vector<float> weights;
        const float average = GetAverageProbabilityWeight();
        for (Edge *e : n->managedEdges) {
            if (!e->mgr->Evaluate()) continue;
            const jet::String id = e->mgr->GetEdgeId();
            float w;
            if (!id.Equals(jet::String::null) && !e->mgr->IsEdgeOnDemand() && HasProbabilityWeight(id)) w = GetProbabilityWeight(id);
            else w = (float)e->mgr->GetProbabilityMultiplier() * average;
            candidates.push_back(e);
            weights.push_back(w);
        }
        Edge *chosen = 0;
        if (!weights.empty()) {
            const std::vector<unsigned> pick = GameUtils::IndexDistributedRandom(weights, 1, 0);
            chosen = candidates[pick[0]];
            chosen->mgr->ResetData();
        }
        for (size_t i = 0; i < candidates.size(); i++) {
            const jet::String id = candidates[i]->mgr->GetEdgeId();
            if (!id.Equals(jet::String::null)) SetProbabilityWeight(id, weights[i]);
        }
        if (chosen) return chosen;
    }
    if (n->pool.empty()) n->pool = n->edges;
    Edge *e = n->pool[(unsigned)GameLevel::s_randomizer[0]->Rand() % n->pool.size()];
    n->pool.erase(std::find(n->pool.begin(), n->pool.end(), e));
    return e;
}

// @0x3e4858: an edge named like a special hub opens its side graph, to return where the edge led
LevelSequenceGraph::Node *LevelSequenceGraph::GetNextNodeByEdge(Edge *e) {
    Node *result;
    auto sp = m_specialNodes.find(e->label);
    if (sp == m_specialNodes.end()) result = m_nodes[e->target];
    else {
        m_isOnSpecialGraph = true;
        m_specialReturn = m_nodes[e->target];
        m_specialEdgeInterval = atoi(e->key.c_str()) - 1;
        result = sp->second->node;
    }
    if (e->mgr) e->mgr->ResetData();
    return result;
}

// @0x3e4940
bool LevelSequenceGraph::IsValidNode(Node *n) const {
    switch (n->type) {
    case Node::RandomHub:
    case Node::SpecialHub: return false;
    case Node::Probability: return (int)((unsigned)GameLevel::s_randomizer[0]->Rand() % 100) < (int)n->probability;
    case Node::Tutorial: return !TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay);
    default: return true;
    }
}

// @0x3e4a18
LevelSequenceGraph::Node *LevelSequenceGraph::JumpToNextNode(Node *n) {
    if (n->type >= Node::RandomHub && n->type <= Node::Tutorial) {
        Node *next = GetNextNodeByEdge(GetNextEdge(n, false, Bifurcation_Center));
        if (n->onCompleted) m_currentNode->onCompleted = n->onCompleted;
        return next;
    }
    if (n->type == Node::SpecialHub) {
        if (ShouldResolveSpecial()) ResolveSpecial();
        return GetSpecialExit(n);
    }
    return 0;
}

// @0x3e4ac8
bool LevelSequenceGraph::ShouldResolveSpecial() const {
    if (m_specialEdgeInterval == 0) return true;
    return m_specialEdgeInterval > 0 && m_specialEdgeInterval < 4 && s_specialExitChoosen[m_specialEdgeInterval] == 0;
}

// @0x3e4b04: two of the four special exits get a type, the others the default
void LevelSequenceGraph::ResolveSpecial() {
    for (int &c : s_specialExitChoosen) c = 3;
    ControlledRandom *r = GameLevel::s_randomizer[0];
    const unsigned a = (unsigned)r->Rand() & 3;
    unsigned b;
    do b = (unsigned)r->Rand() & 3;
    while (a == b);
    s_specialExitChoosen[a] = 1;
    s_specialExitChoosen[b] = 2;
}

// @0x3e4b94
LevelSequenceGraph::Node *LevelSequenceGraph::GetSpecialExit(Node *n) {
    Node *hub = m_nodes[n->id];
    const jet::String *label = &k_defaultStr;
    if ((unsigned)m_specialEdgeInterval < 4) {
        switch (s_specialExitChoosen[m_specialEdgeInterval]) {
        case 1: label = &k_type1Str; break;
        case 2: label = &k_type2Str; break;
        case 3: label = &k_defaultStr; break;
        default: return 0;
        }
    }
    Edge *e = FindEdgeByLabel(hub->edges, *label);
    return m_nodes[e->target];
}

// @0x3e361c
int LevelSequenceGraph::GetNextSequenceNumber() {
    if (!m_sequence) SetLevelSequenceGraphEntryPoint(k_initialEntryPoint);
    return m_sequence->GetNextSequenceNumber();
}

// @0x3e3684: the next template of the node; when it ends the graph goes on to the next node
void LevelSequenceGraph::IncSequenceNumber() {
    const int finished = m_sequence->GetNextSequenceNumber();
    m_sequence->IncSequenceNumber();
    if (m_sequence->GetNextSequenceNumber() != -1) return;
    m_sequence->ResetSequence();
    m_currentNode = GetNextNode(m_currentNode->id, finished);
    m_sequence = m_currentNode->sequence;
}

// @0x3e4da8
void LevelSequenceGraph::Reset() {
    m_sequence = 0;
    m_isOnSpecialGraph = false;
    m_specialReturn = 0;
    m_specialEdgeInterval = -1;
    for (auto &kv : m_nodes) {
        kv.second->pool = kv.second->edges;
        if (kv.second->sequence) kv.second->sequence->Reset();
    }
    for (auto &kv : m_edgeMgrs) kv.second->Reset();
}

// @0x3e4c7c
void LevelSequenceGraph::OnAdvanceLevelTemplateList() {
    for (auto &kv : m_edgeMgrs) kv.second->OnAdvanceLevelTemplateList();
}

// @0x3e4cc0
void LevelSequenceGraph::OnNodeCompleted(const jet::String &id) {
    std::function<void()> f = m_nodes[id]->onCompleted;
    if (f) m_nodes[id]->onCompleted();
}

// @0x3e4d28
void LevelSequenceGraph::OnCompletedTutorial() {
    TutorialMgr::GetInstance()->SetTutorialFinished(ETutorialId::k_firstPlay);
    GameLevel::GetInstance()->OnTutorialCompleted();
}

// @0x3e4e48
bool LevelSequenceGraph::IsNodeInASpecialTree(const jet::String &id) const {
    auto it = m_nodes.find(id);
    return it != m_nodes.end() && it->second->inSpecialTree;
}

// @0x3e4e70: from the start of the graph, following single edges, a tutorial node comes within 10 nodes
bool LevelSequenceGraph::WillPassThroughTutorial() const {
    auto entry = m_locationEntryPoint.find(k_initialEntryPoint);
    const Node *n = m_nodes.at(entry->second);
    std::vector<Edge*> edges = n->edges;
    int hops = 11;
    while (n->type != Node::Tutorial && --hops != 0 && edges.size() == 1) {
        n = m_nodes.at(edges[0]->target);
        edges = n->edges;
    }
    return n->type == Node::Tutorial;
}

// @0x3e4fd0
void LevelSequenceGraph::GetExitPointData(jet::String *label, int *location) {
    ExitPoint *e = m_exitPoints[m_currentNode->id];
    *label = e->label;
    *location = e->location;
}

// @0x3e5124 and on: the graph of a location is read from its file
LevelSequenceGraphMgr::~LevelSequenceGraphMgr() {
    for (auto &kv : m_graphs) delete kv.second;
}

void LevelSequenceGraphMgr::AddLevelSequenceGraph(int location, const jet::String &file) {
    LevelSequenceGraphParser parser;
    m_graphs[location] = parser.CheckAndBuildLevelDescription(file);
}

// @0x3e53f8
void LevelSequenceGraphMgr::SetCurrentLevelSequenceGraph(int location) {
    static const int k_profiles[6] = { 1, 2, 3, 4, 6, 7 };
    if (location >= 0 && location < 6) {
        g_LastActiveColorProfile = g_ActiveColorProfile;
        g_ActiveColorProfile = k_profiles[location];
        g_TransitionTime = 2.0f;
    }
    m_location = location;
    m_current = m_graphs[location];
}

// @0x3e52b8: reaching an exit point sends the run to the graph of another location
void LevelSequenceGraphMgr::IncSequenceNumber() {
    m_current->IncSequenceNumber();
    if (!m_current->IsAtExitPoint()) return;
    jet::String label;
    int location = 0;
    m_current->GetExitPointData(&label, &location);
    SetCurrentLevelSequenceGraph(location);
    m_current->SetLevelSequenceGraphEntryPoint(label);
}

// @0x3e5368
void LevelSequenceGraphMgr::Reset() {
    for (auto &kv : m_graphs)
        if (kv.second) kv.second->Reset();
}
