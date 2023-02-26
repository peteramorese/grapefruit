#include<iostream>

#include "core/Graph.h"

using namespace TP;

struct Edge {
    Edge(float c, const std::string& l) : cost(c), label(l) {}
    bool operator==(const Edge& other) const {return cost == other.cost && label == other.label;}
    float cost;
    std::string label;
    static std::string toStr(const Edge& e) {return "cost: " + std::to_string(e.cost) + " label: " + e.label;}
};

int main() {
 
    Graph<Edge> graph(true, true, &Edge::toStr);
    graph.connect(3, 2, {1.0f, "three to two"});
    graph.connect(3, 3, {2.5f, "three to three"});
    graph.connect(3, 1, {3.4f, "three to one"});
    graph.connect(1, 10, {3.4f, "one to ten"});
    graph.connect(1, 3, {2.5f, "one to three"});
    graph.connect(1, 2, {2.5f, "four to two"});
    graph.print();
    graph.printReverse();

    LOG("Diconnection 3->2");
    graph.disconnect(3, 2, {1.0f, "three to two"});
    graph.print();
    graph.printReverse();

    LOG("Diconnecting all edges from 1 that have cost less than 3.0f");
    auto disconnect_if = [](Node dst, const Edge& edge) -> bool {
        return edge.cost < 3.0f;
    };
    graph.disconnectIf(1, disconnect_if);
    graph.print();
    graph.printReverse();
	return 0;
}
