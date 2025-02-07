#include<iostream>
#include<sstream>

#include "core/Graph.h"

using namespace GF;

struct Edge {
    Edge(float c, const std::string& l) : cost(c), label(l) {}
    bool operator==(const Edge& other) const {return cost == other.cost && label == other.label;}
    float cost;
    std::string label;
    static std::string toStr(const Edge& e) {return "cost: " + std::to_string(e.cost) + " label: " + e.label;}
};

int main() {
 
    Graph<Edge> graph;
    graph.connect(3, 2, {1.0f, "three to two"});
    graph.connect(3, 3, {1.9f, "three to three"});
    graph.connect(3, 1, {3.4f, "three to one"});
    graph.connect(1, 10, {3.4f, "one to ten"});
    graph.connect(1, 3, {1.5f, "one to three"});
    graph.connect(1, 2, {2.5f, "one to two"});
    graph.connect(5, 2, {2.5f, "five to two"});
    graph.print(&Edge::toStr);
    graph.rprint(&Edge::toStr);

    std::string nodes_str = "";
    for (auto n : graph.nodes()) nodes_str += std::to_string(n) + " ";
    LOG("Nodes: " << nodes_str);

    //LOG("Diconnection 3->2");
    //graph.disconnect(3, 2, {1.0f, "three to two"});
    //graph.print();
    //graph.printReverse();

    //const auto& children = graph.getChildren(1); // demonstrate pointer stability
    //LOG("Diconnecting all edges from 1 that have cost less than 3.0f (size before: " << children.size() << ")");
    //auto disconnect_if = [](Node dst, const Edge& edge) -> bool {
    //    return edge.cost < 3.0f;
    //};
    //graph.disconnectIf(1, disconnect_if);
    //LOG("(size after: " << children.size() << ")");

    //graph.print();
    //graph.printReverse();
	return 0;
}
