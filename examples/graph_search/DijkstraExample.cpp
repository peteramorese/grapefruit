#include<iostream>
#include<string>
#include<sstream>
#include<memory>

#include "tools/Logging.h"
#include "core/Graph.h"
#include "graph_search/SearchProblem.h"
#include "graph_search/AStar.h"

using namespace TP;
using namespace TP::GraphSearch;

struct Edge {
    // Edge does not need to be default constructable
    Edge() = delete;

    Edge(uint32_t cost_, char edge_action_) : cost(cost_), edge_action(edge_action_) {}
    bool operator==(const Edge& other) const {return cost == other.cost && edge_action == other.edge_action;}
    uint32_t cost = 0;
    char edge_action = '\0';

    operator uint32_t() const {return cost;}
    operator uint32_t&&() {return std::move(cost);}

    static uint32_t edgeToCost(const Edge& edge) {return edge.cost;}
    static std::string edgeToStr(const Edge& edge) {return "cost: " + std::to_string(edge.cost) + " edge action: " + edge.edge_action;}
};

int main() {

    std::shared_ptr<Graph<Edge>> graph(std::make_shared<Graph<Edge>>());

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

    graph->print(&Edge::edgeToStr);
 

    NEW_LINE;
    LOG("Default search example");
    {
    QuantitativeGraphSearchProblem<Graph<Edge>, uint32_t, SearchDirection::Forward> dijkstras_problem(graph, {1}, {6});

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
    QuantitativeGraphSearchProblem<Graph<Edge>, uint32_t, SearchDirection::Forward> dijkstras_problem(graph, {1}, {6});

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
