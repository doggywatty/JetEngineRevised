// source/game/leveltemplate/LevelSequenceGraphParser.cpp
#include "game/leveltemplate/LevelSequenceGraphParser.h"
#include <cstring>
#include "game/Player.h"
#include "game/leveltemplate/LevelSequenceGraph.h"
#include "game/leveltemplate/LevelSequenceParser.h"
#include "stream/istream.h"
#include "stream/streammgr.h"

// @0x3e55d8: the first time the player plays the lab graph its first rooms are a fixed list
LevelSequenceGraph *LevelSequenceGraphParser::CheckAndBuildLevelDescription(const jet::String &file) {
    pugi::xml_document doc;
    LoadGraph(file, doc);
    m_graph = new LevelSequenceGraph;
    m_parser = new LevelSequenceParser;
    m_graph->SetName(file);
    if (!Player::GetInstance()->IsSideLookTutorialDone() && file.Equals("levelsequence2_labo.graphml")) {
        pugi::xml_node graph = doc.child("graphml").child("graph");
        static const char *const k_nodes[3] = { "n19", "n133", "n169" };
        for (const char *id : k_nodes) {
            pugi::xml_node d5 = graph.find_child_by_attribute("id", id).find_child_by_attribute("key", "d5");
            d5.last_child().set_value("[Location{laboRoom},[[Camera{@CameraBack, 1},45,Camera{@CameraSide,1},344,Camera{@CameraSide,1},345]/1]/R,Camera{@CameraBack, 1000}]/1");
        }
    }
    doc.traverse(*this);
    if (!m_validGraph) {
        delete m_graph;
        m_graph = 0;
    }
    delete m_parser;
    m_parser = 0;
    return m_graph;
}

// @0x3e58bc
bool LevelSequenceGraphParser::for_each(pugi::xml_node &n) {
    if (!strcmp(n.name(), "node")) ProcessNode(n);
    if (!strcmp(n.name(), "edge")) ProcessEdge(n);
    return m_validGraph;
}

// @0x3e594c
void LevelSequenceGraphParser::LoadGraph(const jet::String &file, pugi::xml_document &doc) {
    m_file = file;
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(m_file);
    if (!s) return;
    s->Start();
    const unsigned n = s->Size();
    if (n) {
        char *buf = new char[n];
        memset(buf, 0, n);
        s->Read(buf, n);
        doc.load_buffer(buf, n, pugi::parse_default, pugi::encoding_auto);
        delete[] buf;
    }
    s->Finish();
    delete s;
}

// @0x3e5fb4: a node of a folder (an id with a ':') and the group itself are left out
bool LevelSequenceGraphParser::ShouldNodeBeProcessed(pugi::xml_node &n) {
    const bool nested = strchr(n.attribute("id").value(), ':') != 0;
    return !nested && strcmp(n.attribute("yfiles.foldertype").value(), "group") != 0;
}

// @0x3e6410
bool LevelSequenceGraphParser::ShouldEdgeBeProcessed(pugi::xml_node &n) {
    return !strchr(n.attribute("source").value(), ':') && !strchr(n.attribute("target").value(), ':');
}

// @0x3e5a4c
void LevelSequenceGraphParser::ProcessNode(pugi::xml_node &n) {
    const char *id = n.attribute("id").value();
    const char *text = "", *label = "";
    bool ellipse = false, diamond = false, triangle = false;
    for (pugi::xml_node d = n.child("data"); d; d = d.next_sibling("data")) {
        const char *key = d.attribute("key").value();
        if (!strcmp(key, "d5")) text = d.first_child().value();
        else if (!strcmp(key, "d6")) {
            pugi::xml_node shapeNode = d.child("y:ShapeNode");
            const char *type = shapeNode.child("y:Shape").attribute("type").value();
            if (!strcmp(type, "ellipse")) ellipse = true;
            else if (!strcmp(type, "diamond")) diamond = true;
            else if (!strcmp(type, "triangle")) triangle = true;
            label = shapeNode.child("y:NodeLabel").first_child().value();
        }
    }
    if (!ShouldNodeBeProcessed(n)) return;
    if (*text && ellipse) m_graph->AddEntryPoint(id, label, text);
    else if (diamond) m_graph->AddRandomHub(id, label);
    // a special hub is found by its label (the edges that open it carry it, "SpLab0"); its text is empty, which made every plain edge open it
    else if (triangle) m_graph->AddSpecialHub(id, label, label);
    else if (*text) {
        const jet::String s(text);
        if (m_parser->IsValidProbabilityFunction(s)) {
            int percent = 0;
            LevelSequence *seq = 0;
            m_parser->ProcessProbabilityFunction(s, &percent, &seq);
            m_graph->AddProbabilityNode(id, label, seq, percent);
        } else if (m_parser->IsValidTutorialFunction(s)) {
            LevelSequence *seq = 0;
            m_parser->ProcessTutorialFunction(s, &seq);
            m_graph->AddTutorialNode(id, label, seq);
        } else if (m_parser->IsValidSetTutorialFinishedFunction(s)) {
            m_graph->AddTutorialFinishedNode(id, label);
        } else if (m_parser->IsValidExitFunction(s)) {
            jet::String toLabel;
            ELocation location;
            m_parser->ProcessExitFunction(s, &toLabel, &location);
            m_graph->AddExitPoint(id, label, toLabel, (int)location);
        } else if (m_parser->CheckLevelString(s)) {
            m_graph->AddNode(id, label, m_parser->BuildLevelDescription(s));
        }
    }
}

// @0x3e608c
void LevelSequenceGraphParser::ProcessEdge(pugi::xml_node &n) {
    const char *id = n.attribute("id").value(), *source = n.attribute("source").value(), *target = n.attribute("target").value();
    const char *label = "", *key = "";
    bool probability = false, conditional = false;
    for (pugi::xml_node d = n.child("data"); d; d = d.next_sibling("data")) {
        const char *k = d.attribute("key").value();
        if (!strcmp(k, "d9")) key = d.first_child().value();
        else if (!strcmp(k, "d10")) {
            pugi::xml_node line = d.child("y:PolyLineEdge");
            label = line.child("y:EdgeLabel").first_child().value();
            pugi::xml_node arrows = line.child("y:Arrows");
            const char *a = arrows.attribute("source").value(), *b = arrows.attribute("target").value();
            if (!strcmp(a, "crows_foot_optional") || !strcmp(b, "crows_foot_optional")) probability = true;
            else if (!strcmp(a, "crows_foot_one_mandatory") || !strcmp(b, "crows_foot_one_mandatory")) conditional = true;
        }
    }
    if (!ShouldEdgeBeProcessed(n)) return;
    if (probability) m_graph->AddProbabilityEdge(id, label, key, source, target, *m_parser);
    else if (conditional) m_graph->AddConditionalEdge(id, label, key, source, target, *m_parser);
    else m_graph->AddEdge(id, label, key, source, target);
}
