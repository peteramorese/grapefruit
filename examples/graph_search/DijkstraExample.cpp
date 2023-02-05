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
    uint32_t cost = 0;
    char edge_action = '\0';

    static uint32_t edgeToCost(const Edge& edge) {return edge.cost;}
    static std::string edgeToStr(const Edge& edge) {return "cost: " + std::to_string(edge.cost) + " edge action: " + edge.edge_action;}
};

int main() {

    std::shared_ptr<Graph<Edge>> graph(std::make_shared<Graph<Edge>>(true, true, &Edge::edgeToStr));

    graph->connect(0, 3, {1, 'b'});
    graph->connect(1, 0, {2, 'a'});
    graph->connect(1, 2, {10, 'h'});
    graph->connect(1, 4, {11, 'm'});
    graph->connect(2, 4, {8, 'i'});
    graph->connect(2, 5, {15, 'j'});
    graph->connect(3, 4, {3, 'e'});
    graph->connect(4, 5, {2, 'd'});
    graph->connect(4, 7, {9, 'k'});
    graph->connect(4, 6, {7, 'l'});
    graph->connect(5, 8, {2, 'e'});
    graph->connect(7, 6, {3, 'g'});
    graph->connect(8, 7, {1, 'f'});
 
    QuantitativeGraphSearchProblem<Edge, uint32_t> dijkstras_problem(graph, 1, 6, &Edge::edgeToCost);

    LOG("Searching...");
    auto result = AStar<Node, Edge, uint32_t, decltype(dijkstras_problem)>::search(dijkstras_problem);

    LOG("Finished!");
    LOG("Success: " << ((result.success) ? "true" : "false"));
    if (result.success) {
        LOG("Path length: " << result.path_cost);

        std::stringstream path_str;
        path_str << std::to_string(result.node_path.front());
        auto edge_path_it = result.edge_path.begin();
        //for (auto it = ++result.node_path.begin(); it != result.node_path.end(); ++it) path_str << (" --" << (edge_path_it++->edge_action) << "-> " << std::to_string(*it));

        //LOG("Path: " << path_str); 
    }

	return 0;
}
