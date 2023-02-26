#include<iostream>
#include<string>
#include<sstream>
#include<memory>

#include "tools/Logging.h"
#include "core/Graph.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "graph_search/NAMOAStar.h"

using namespace TP;
using namespace TP::GraphSearch;

struct Edge {
    // Edge does not need to be default constructable
    Edge() = delete;

    Edge(uint32_t cost_1, uint32_t cost_2, char edge_action_) : cv({cost_1, cost_2}), edge_action(edge_action_) {}
    bool operator==(const Edge& other) const {return cv == other.cv && edge_action == other.edge_action;}
    Containers::FixedArray<2, uint32_t> cv;
    char edge_action = '\0';

    
    static Containers::FixedArray<2, uint32_t> edgeToCostVector(const Edge& edge) {return edge.cv;}
    static std::string cvToStr(const Containers::FixedArray<2, uint32_t>& cv) {return "(" + std::to_string(cv[0]) + ", " + std::to_string(cv[1]) + ")";}
    static std::string edgeToStr(const Edge& edge) {return "cost: " + cvToStr(edge.cv) + " edge action: " + edge.edge_action;}
};

int main() {

    std::shared_ptr<Graph<Edge>> graph(std::make_shared<Graph<Edge>>(true, true, &Edge::edgeToStr));

    graph->connect(0, 1, {6, 1, 'a'});
    graph->connect(0, 2, {2, 1, 'b'});
    graph->connect(1, 3, {1, 9, 'c'});
    graph->connect(1, 6, {3, 8, 'd'});
    graph->connect(1, 4, {3, 2, 'e'});
    graph->connect(2, 4, {1, 4, 'f'});
    graph->connect(2, 5, {6, 1, 'g'});
    graph->connect(2, 7, {8, 9, 'h'});
    graph->connect(3, 6, {2, 3, 'i'});
    graph->connect(4, 6, {6, 4, 'j'});
    graph->connect(4, 7, {1, 5, 'k'});
    graph->connect(5, 7, {1, 1, 'l'});

    graph->print();
 

    NEW_LINE;
    LOG("Default Bi-Objective search example");
    {
    MOQuantitativeGraphSearchProblem<Graph<Edge>, Containers::FixedArray<2, uint32_t>, SearchDirection::Forward> dijkstras_problem(graph, {0}, {7}, &Edge::edgeToCostVector);

    NEW_LINE;
    LOG("Searching...");
    auto result = NAMOAStar<Containers::FixedArray<2, uint32_t>, decltype(dijkstras_problem)>::search(dijkstras_problem);

    LOG("Finished!");
    LOG(((result.success) ? "Found path (success)" : "Did not find path (failure)"));
    if (result.success) {
        uint32_t pt = 0;
        for (const auto& solution : result.solution_set) {
            LOG("Pareto point " << pt++);
            LOG("   Path length: " << Edge::cvToStr(solution.path_cost));

            //std::string path_str = std::to_string(solution.node_path.front());
            //auto edge_path_it = solution.edge_path.begin();
            //for (auto it = ++solution.node_path.begin(); it != solution.node_path.end(); ++it) {
            //    path_str += " --(";
            //    path_str.push_back(edge_path_it++->edge_action);
            //    path_str += ")-> " + std::to_string(*it);
            //}

            //LOG("   Path: " << path_str); 

        }
    }
    }

	return 0;
}
