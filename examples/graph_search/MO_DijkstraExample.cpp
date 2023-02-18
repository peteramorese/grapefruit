#include<iostream>
#include<string>
#include<sstream>
#include<memory>

#include "tools/Logging.h"
#include "core/Graph.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "graph_search/BOAStar.h"

using namespace TP;
using namespace TP::GraphSearch;

struct Edge {
    // Edge does not need to be default constructable
    Edge() = delete;

    Edge(uint32_t cost_1, uint32_t cost_2, char edge_action_) : cv({cost_1, cost_2}), edge_action(edge_action_) {}
    CostVector<2, uint32_t> cv;
    char edge_action = '\0';

    static CostVector<2, uint32_t> edgeToCostVector(const Edge& edge) {return edge.cv;}
    static std::string edgeToStr(const Edge& edge) {return "cost: (" + std::to_string(edge.cv[0]) + ", " + std::to_string(edge.cv[1]) + ") edge action: " + edge.edge_action;}
};

int main() {

    std::shared_ptr<Graph<Edge>> graph(std::make_shared<Graph<Edge>>(true, true, &Edge::edgeToStr));

    graph->connect(0, 3, {1, 'b'});
    graph->connect(1, 0, {2, 'a'});
    graph->connect(1, 2, {10, 'h'});
    graph->connect(1, 4, {11, 'm'});
    graph->connect(2, 4, {8, 'i'});
    graph->connect(2, 5, {15, 'j'});
    graph->connect(3, 4, {3, 'c'});
    graph->connect(4, 5, {2, 'd'});
    graph->connect(4, 7, {9, 'k'});
    graph->connect(4, 6, {17, 'l'});
    graph->connect(5, 8, {2, 'e'});
    graph->connect(7, 6, {3, 'g'});
    graph->connect(8, 7, {1, 'f'});

    graph->print();
 

    NEW_LINE;
    LOG("Default search example");
    {
    QuantitativeGraphSearchProblem<Edge, uint32_t, SearchDirection::Forward> dijkstras_problem(graph, {1}, 6, &Edge::edgeToCost);

    NEW_LINE;
    LOG("Searching...");
    auto result = AStar<Node, Edge, uint32_t, decltype(dijkstras_problem)>::search(dijkstras_problem);

    LOG("Finished!");
    LOG(((result.success) ? "Found path (success)" : "Did not find path (failure)"));
    if (result.success) {
        LOG("Path length: " << result.solution.path_cost);

        std::string path_str = std::to_string(result.solution.node_path.front());
        auto edge_path_it = result.solution.edge_path.begin();
        for (auto it = ++result.solution.node_path.begin(); it != result.solution.node_path.end(); ++it) {
            path_str += " --(";
            path_str.push_back(edge_path_it++->edge_action);
            path_str += ")-> " + std::to_string(*it);
        }

        LOG("Path: " << path_str); 
    }
    }

    NEW_LINE;
    LOG("Memory minimal example");
    {
    QuantitativeGraphSearchProblem<Edge, uint32_t, SearchDirection::Forward> dijkstras_problem(graph, {1}, 6, &Edge::edgeToCost);

    LOG("Searching...");
    auto result = AStar<Node, Edge, uint32_t, decltype(dijkstras_problem), ZeroHeuristic<Node, uint32_t>, const Edge*>::search(dijkstras_problem);

    LOG("Finished!");
    LOG(((result.success) ? "Found path (success)" : "Did not find path (failure)"));
    if (result.success) {
        LOG("Path length: " << result.solution.path_cost);

        std::string path_str = std::to_string(result.solution.node_path.front());
        auto edge_path_it = result.solution.edge_path.begin();
        for (auto it = ++result.solution.node_path.begin(); it != result.solution.node_path.end(); ++it) {
            path_str += " --(";

            // Memory minimal storage stores persistent const ptrs to the edge inside of the explicit graph data structure instead of copies
            const Edge* edge_ptr = *(edge_path_it++);
            path_str.push_back((edge_ptr)->edge_action);

            path_str += ")-> " + std::to_string(*it);
        }

        LOG("Path: " << path_str); 
    }
    }

	return 0;
}
