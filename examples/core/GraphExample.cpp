#include<iostream>

#include "core/Graph.h"

using namespace TP;

int main() {
 
    struct Edge {
        Edge(float c, const std::string& l) : cost(c), label(l) {}
        float cost;
        std::string label;
        static std::string toStr(const Edge& e) {return "cost: " + std::to_string(e.cost) + " label: " + e.label;}
    };
    Graph<Edge> graph(true, true, &Edge::toStr);
    graph.connect(3, 2, {1.0f, "three to two"});
    graph.connect(3, 3, {2.5f, "three to three"});
    graph.connect(0, 10, {3.4f, "zero to ten"});
    graph.print();
    graph.printReverse();
	return 0;
}
