// source/game/leveltemplate/LevelSequenceGraph.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCEGRAPH_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCEGRAPH_H

#include <functional>
#include <map>
#include <set>
#include <vector>
#include "core/pstring.h"
#include "game/gameenums.h"
#include "game/leveltemplate/LevelSequence.h"
#include "game/leveltemplate/SpeedDef.h"

class EdgeMgr;
class LevelSequenceParser;

// The run of a location drawn as a graph (a yEd file): the nodes hold a level description, the edges are followed at random or by a
// condition or a probability. Hubs pick an edge without a template, tutorial nodes only come until the tutorial is finished, special
// hubs open a side graph and exit points lead to the graph of another location.
class LevelSequenceGraph {  // sizeof 0xA0
public:
    struct Edge {
        jet::String source, target, label, key;  // 0x0, 0x4, 0x8, 0xC
        EdgeMgr *mgr;                            // 0x10
        Edge(const char *label_, const char *key_, const char *source_, const char *target_) : source(source_), target(target_), label(label_), key(key_), mgr(0) {}
    };
    struct Node {
        enum Type { Normal = 0, RandomHub = 1, Probability = 2, Tutorial = 3, SpecialHub = 4, ExitPoint = 5 };
        int type;                          // 0x0
        LevelSequence *sequence;           // 0x4
        jet::String label, id;             // 0x8, 0xC
        unsigned char probability;         // 0x10: percent of a probability node
        std::vector<Edge*> edges;          // 0x14: every edge leaving it
        std::vector<Edge*> pool;           // 0x20: those not taken yet
        std::vector<Edge*> managedEdges;   // 0x2C: those a manager decides
        std::function<void()> onCompleted; // 0x38
        bool inSpecialTree;                // 0x48
        Node(const char *id_, const char *label_, LevelSequence *s) : type(Normal), sequence(s), label(label_), id(id_), probability(0), inSpecialTree(false) {}
    };
    struct SpecialNode {
        Node *node;
        unsigned unused;
    };
    struct ExitPoint {
        jet::String label;
        int location;
    };
    static const jet::String k_initialEntryPoint, k_ProbabilityEvolutionSet_tmplName, k_ProbabilityIntervalDef_tmplName;
    LevelSequenceGraph() : m_sequence(0), m_currentNode(0), m_isOnSpecialGraph(false), m_specialReturn(0), m_specialEdgeInterval(-1) {}
    ~LevelSequenceGraph();
    // building (the parser)
    void AddNode(const char *id, const char *label, LevelSequence*);
    void AddTutorialNode(const char *id, const char *label, LevelSequence*);
    void AddExitPoint(const char *id, const char *label, const jet::String &toLabel, int location);
    void AddRandomHub(const char *id, const char *label);
    void AddTutorialFinishedNode(const char *id, const char *label);
    void AddSpecialHub(const char *id, const char *label, const char *name);
    void AddProbabilityNode(const char *id, const char *label, LevelSequence*, int percent);
    void AddEntryPoint(const char *id, const char *label, const char *name);
    void AddEdge(const char *edgeId, const char *label, const char *key, const char *source, const char *target);
    void AddProbabilityEdge(const char *edgeId, const char *label, const char *key, const char *source, const char *target, LevelSequenceParser&);
    void AddConditionalEdge(const char *edgeId, const char *label, const char *key, const char *source, const char *target, LevelSequenceParser&);
    void SetName(const jet::String &n) { m_name = n; }
    // the run
    void SetLevelSequenceGraphEntryPoint(const jet::String &name);
    int GetNextSequenceNumber();
    void IncSequenceNumber();
    void Reset();
    const jet::String &GetCameraName() { return m_sequence->GetCameraName(); }
    int GetCameraTransitionTime() { return m_sequence->GetCameraTransitionTime(); }
    SpeedDef *GetSpeed() { return m_sequence->GetSpeed(); }
    const jet::String &GetLocation() { return m_sequence->GetLocation(); }
    const jet::String &GetNextNodeLabel() { return m_currentNode->label; }
    const jet::String &GetNextNodeId() { return m_currentNode->id; }
    void OnNodeCompleted(const jet::String &id);
    void OnCompletedTutorial();
    void OnAdvanceLevelTemplateList();
    bool IsNodeInASpecialTree(const jet::String &id) const;
    bool WillPassThroughTutorial() const;
    void GetExitPointData(jet::String *label, int *location);
    bool IsAtExitPoint() const { return m_exitPoints.find(m_currentNode->id) != m_exitPoints.end(); }
    // the weights edges are chosen by, shared between graphs
    static void SetProbabilityWeight(const jet::String &id, float w);
    static float GetProbabilityWeight(const jet::String &id);
    static bool HasProbabilityWeight(const jet::String &id) { return s_probabilityMap.find(id) != s_probabilityMap.end(); }
    static float GetAverageProbabilityWeight();
    static void ResetStaticData();
private:
    Node *GetNextNode(const jet::String &id, int finishedTemplate);
    Edge *GetNextEdge(Node*, bool useBifurcation, Bifurcation);
    Node *GetNextNodeByEdge(Edge*);
    bool IsValidNode(Node*) const;
    Node *JumpToNextNode(Node*);
    bool ShouldResolveSpecial() const;
    void ResolveSpecial();
    Node *GetSpecialExit(Node*);
    const jet::String &GetDesiredLabel(Bifurcation) const;
    Edge *FindEdgeByLabel(const std::vector<Edge*>&, const jet::String &label) const;
    void InitLevelSequenceGraph();
    void MarkNodeAsInSpecialTree(Node*);
    jet::String m_name;                                    // 0x4
    std::map<jet::String, Node*> m_nodes;                  // 0x8
    std::map<jet::String, jet::String> m_locationEntryPoint;  // 0x20: entry point name -> node id
    std::vector<ExitPoint*> m_exitPointList;               // 0x38
    std::map<jet::String, ExitPoint*> m_exitPoints;        // 0x44
    std::map<jet::String, SpecialNode*> m_specialNodes;    // 0x5C
    std::map<jet::String, EdgeMgr*> m_edgeMgrs;            // 0x74
    LevelSequence *m_sequence;                             // 0x8C
    Node *m_currentNode;                                   // 0x90
    bool m_isOnSpecialGraph;                               // 0x94
    Node *m_specialReturn;                                 // 0x98
    int m_specialEdgeInterval;                             // 0x9C
    static const jet::String k_leftStr, k_rightStr, k_type1Str, k_type2Str, k_defaultStr;
    static std::map<jet::String, float> s_probabilityMap;
    static float s_totalProbabilityWeight;
    static int s_specialExitChoosen[4];
};

// The graphs of the locations and which one is current
class LevelSequenceGraphMgr {  // sizeof 0x24
public:
    LevelSequenceGraphMgr() : m_location(0), m_current(0) {}
    ~LevelSequenceGraphMgr();
    void AddLevelSequenceGraph(int location, const jet::String &file);
    bool HasLevelSequenceGraph(int location) const { return m_graphs.find(location) != m_graphs.end(); }
    // switching the location also starts the colour profile of the location
    void SetCurrentLevelSequenceGraph(int location);
    int GetNextNodeGraphId() const { return m_location; }
    const jet::String &GetLocation() { return m_current->GetLocation(); }
    const jet::String &GetNextNodeLabel() { return m_current->GetNextNodeLabel(); }
    const jet::String &GetNextNodeId() { return m_current->GetNextNodeId(); }
    int GetNextSequenceNumber() { return m_current->GetNextSequenceNumber(); }
    void IncSequenceNumber();
    void Reset();
    const jet::String &GetCameraName() { return m_current->GetCameraName(); }
    int GetCameraTransitionTime() { return m_current->GetCameraTransitionTime(); }
    SpeedDef *GetSpeed() { return m_current->GetSpeed(); }
    void OnAdvanceLevelTemplateList() { m_current->OnAdvanceLevelTemplateList(); }
    void OnNodeCompleted(int location, const jet::String &id) { m_graphs[location]->OnNodeCompleted(id); }
    void OnCompletedTutorial() { m_current->OnCompletedTutorial(); }
    bool IsNodeInASpecialTree(const jet::String &id) const { return m_current->IsNodeInASpecialTree(id); }
    bool WillPassThroughTutorial() const { return m_current->WillPassThroughTutorial(); }
private:
    int m_location;                              // 0x4
    LevelSequenceGraph *m_current;               // 0x8
    std::map<int, LevelSequenceGraph*> m_graphs; // 0xC
};

#endif
