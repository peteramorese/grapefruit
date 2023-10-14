#include<iostream>
#include<string>
#include<sstream>
#include<memory>

#include "tools/Logging.h"
#include "core/Graph.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "graph_search/BOAStar.h"

using namespace GF;
using namespace GF::GraphSearch;

struct Edge {
    // Edge does not need to be default constructable
    Edge() = delete;
    bool operator==(const Edge& other) const {return cv == other.cv && edge_action == other.edge_action;}

    Edge(uint32_t cost_1, uint32_t cost_2, char edge_action_) : cv({cost_1, cost_2}), edge_action(edge_action_) {}
    Containers::FixedArray<2, uint32_t> cv;
    char edge_action = '\0';

    
    static Containers::FixedArray<2, uint32_t> edgeToCostVector(const Edge& edge) {return edge.cv;}
    static std::string cvToStr(const Containers::FixedArray<2, uint32_t>& cv) {return "(" + std::to_string(cv[0]) + ", " + std::to_string(cv[1]) + ")";}
    static std::string edgeToStr(const Edge& edge) {return "cost: " + cvToStr(edge.cv) + " edge action: " + edge.edge_action;}
};

class MyHeuristic {
    public:
        Containers::FixedArray<2, uint32_t> operator()(Node node) const {return m_heuristic_values.at(node);}
        std::map<Node, Containers::FixedArray<2, uint32_t>> m_heuristic_values; 
};

int main() {

    std::shared_ptr<Graph<Edge>> graph(std::make_shared<Graph<Edge>>());

    graph->connect(0, 1, {0, 5, 'a'});
    graph->connect(0, 2, {4, 1, 'b'});
    graph->connect(1, 2, {2, 2, 'c'});
    graph->connect(1, 3, {0, 5, 'd'});
    graph->connect(2, 3, {3, 4, 'f'});
    graph->connect(2, 4, {5, 1, 'e'});
    graph->connect(4, 3, {3, 2, 'g'});

    graph->print(&Edge::edgeToStr);
 

    NEW_LINE;
    LOG("Default Bi-Objective search example");
    {
    MOQuantitativeGraphSearchProblem<Graph<Edge>, Containers::FixedArray<2, uint32_t>, SearchDirection::Forward, MyHeuristic> astar_problem(graph, {0}, {3}, &Edge::edgeToCostVector);

    // Manually insert heuristic values (i.e. integer min number of edges to goal):
    MyHeuristic& heuristic = astar_problem.heuristic;
    heuristic.m_heuristic_values[0] = {{0, 2}};
    heuristic.m_heuristic_values[1] = {{0, 1}};
    heuristic.m_heuristic_values[2] = {{1, 1}};
    heuristic.m_heuristic_values[3] = {{0, 0}};
    heuristic.m_heuristic_values[4] = {{1, 0}};

    NEW_LINE;
    LOG("Searching...");
    auto result = BOAStar<Containers::FixedArray<2, uint32_t>, decltype(astar_problem)>::search(astar_problem);

    LOG("Finished!");
    LOG(((result.success) ? "Found path (success)" : "Did not find path (failure)"));
    if (result.success) {
        uint32_t pt = 0;
        auto pf_it = result.pf.begin();
        for (const auto& solution : result.solution_set) {
            LOG("Pareto point " << pt++);
            LOG("   Path length: " << Edge::cvToStr(*pf_it++));

            std::string path_str = std::to_string(solution.node_path.front());
            auto edge_path_it = solution.edge_path.begin();
            for (auto it = ++solution.node_path.begin(); it != solution.node_path.end(); ++it) {
                path_str += " --(";
                path_str.push_back(edge_path_it++->edge_action);
                path_str += ")-> " + std::to_string(*it);
            }

            LOG("   Path: " << path_str); 

        }
    }
    }

	return 0;
}
