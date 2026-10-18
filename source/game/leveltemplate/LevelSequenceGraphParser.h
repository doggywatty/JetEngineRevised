// source/game/leveltemplate/LevelSequenceGraphParser.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCEGRAPHPARSER_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELSEQUENCEGRAPHPARSER_H

#include "core/pstring.h"
#include "pugixml.hpp"

class LevelSequenceGraph;
class LevelSequenceParser;

// Reads the yEd file of a location's graph: the shape of a node says what it is (an ellipse with a name is an entry point, a diamond a
// random hub, a triangle a special hub), its data holds the level description, an edge's label and arrows say how it is taken
class LevelSequenceGraphParser : public pugi::xml_tree_walker {  // sizeof 0x18
public:
    LevelSequenceGraphParser() : m_graph(0), m_parser(0), m_validGraph(true) {}
    virtual ~LevelSequenceGraphParser() {}
    // null when the file does not hold a valid graph
    LevelSequenceGraph *CheckAndBuildLevelDescription(const jet::String &file);
    virtual bool for_each(pugi::xml_node &n);
private:
    void LoadGraph(const jet::String &file, pugi::xml_document &doc);
    void ProcessNode(pugi::xml_node&);
    void ProcessEdge(pugi::xml_node&);
    bool ShouldNodeBeProcessed(pugi::xml_node&);
    bool ShouldEdgeBeProcessed(pugi::xml_node&);
    jet::String m_file;               // 0x8
    LevelSequenceGraph *m_graph;      // 0x10
    LevelSequenceParser *m_parser;    // 0x14
    bool m_validGraph;                // 0xC
};

#endif
