#pragma once

#include <array>
#include <queue>
#include <map>
#include <map>
#include <memory>
#include <algorithm>
#include <type_traits>

#include "tools/Logging.h"
#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "core/Graph.h"

#define TP_COST_VECTOR_EQUIVALENCE_TOLERANCE 0.0000000001



namespace TP {

// COST_T currently only requires operator<
template <typename T>
T abs(const T& x) {return (x < T{}) ? -x : x;}

namespace GraphSearch {


    template <class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, CostVector<2, COST_T>>, typename EDGE_STORAGE_T = EDGE_T>
    class BOAStar {
        public:
            static MultiObjectiveSearchResult<2, EDGE_STORAGE_T, COST_T> search(const SEARCH_PROBLEM_T& problem);

        private:
            using EnumeratedNode = int32_t;
            
            template<class EDGE_STORAGE_T>
            class PathEnumeratedNodeMap {
                private:
                    struct Value {
                        Node node;
                        EnumeratedNode parent;
                        EDGE_STORAGE_T parent_edge;
                    };
                public:
                    inline bool hasParent(EnumeratedNode enumerated_node;) const {return map[enumerated_node].parent != s_null;}
                    inline Node getNode(EnumeratedNode enumerated_node;) const {return map[enumerated_node].node;}
                    inline EnumeratedNode getParentEnumeratedNode(EnumeratedNode enumerated_node;) const {return map[enumerated_node].parent;}
                    inline const EDGE_STORAGE_T& getParentEdge(EnumeratedNode enumerated_node;) const {return map[enumerated_node].parent_edge;}
                    inline EnumeratedNode newNode(Node node, EnumeratedNode parent) {
                        enum_node_to_node.push_back({node, parent}); 
                        return enum_node_to_node.size() - 1;
                    }
                    inline EnumeratedNode newInitNode(Node node) {return newNode(node, s_null);}
                private:
                    inline static EnumeratedNode s_null = -1;
                    std::vector<std::pair<Node, EnumeratedNode>> map; // Maps enum node key to the actual node and the parent enum node
            };

        private:
            static void extractPath(const EnumeratedNode& goal_node, PathSolution<Node, EDGE_STORAGE_T, COST_T>& path_solution, const PathEnumeratedNodeMap<EDGE_STORAGE_T>& node_map);
    };

    template <class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    MultiObjectiveSearchResult<2, EDGE_STORAGE_T, COST_T> BOAStar<EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::search(const SEARCH_PROBLEM_T& problem) {
        using CV_T = CostVector<2, COST_T>;

        // If custom edge storage type is used with explicit search, assert that the outgoingEdges method is explicit (returns a persistent const reference)
        constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, EDGE_T>::value;
        if constexpr (_PTR_EDGE_STORAGE_TYPE) {
            using _EDGE_CONTAINER_T = std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, NODE_T)>::type;
            constexpr bool _IS_EXPLICIT = std::is_reference<_EDGE_CONTAINER_T>();
            static_assert(_IS_EXPLICIT, "Must use explicit graph construction with non-default EDGE_STORAGE_T");
            static_assert(std::is_same<EDGE_STORAGE_T, const EDGE_T*>::value, "EDGE_STORAGE_T must be a persistent const COST_T pointer");
        }

        // Instantiate return value (search graph and non-dominated cost map are allocated, but not used for this algorithm)
        MultiObjectiveSearchResult<2, EDGE_STORAGE_T, COST_T> result(false, false);



        // Open set
        struct OpenSetElement {
            OpenSetElement(const EnumeratedNode& node_, const CV_T& g_score_, const CV_T& f_score_) : node(node_), g_score(g_score_), f_score(f_score_) {}
            EnumeratedNode node; 
            CV_T g_score; // g_score at the time of insertion ()
            CV_T f_score; // f_score (g_score + h)
        };
        auto less = [](const OpenSetElement& lhs, const OpenSetElement& rhs) -> bool {return rhs.f_score < lhs.f_score;}; // Open set element lexicographic comparator (lhs and rhs are swapped for increasing order)
        std::priority_queue<OpenSetElement, std::vector<OpenSetElement>, decltype(less)> open_set;

        // Enum node map (maps 'nodes' AKA EnumeratedNode to 'states' AKA Node in BOA), includes parent map
        PathEnumeratedNodeMap<EDGE_STORAGE_T> path_enum_node_map;

        // G-score container (g_2 min cost map for second objective)
        MinCostMap<Node, COST_T> g_2_min;
        for (const auto& init_node : problem.initial_node_set) {
            EnumeratedNode init_enum_node = path_enum_node_map.newInitNode(init_node);

            // Add initial node to open set
            open_set.emplace(problem.initial_node, CV_T{}, problem.hScore(problem.initial_node));
        }

        // Keep track of the min cost to any goal node
        bool g_2_min_goal_set = false;
        COST_T g_2_min_goal = COST_T{};


        while (!open_set.empty()) {
            auto[curr_enum_node, inserted_g_score, inserted_f_score] = open_set.top();
            Node curr_node = path_enum_node_map.getNode(curr_enum_node);
            open_set.pop(); 
            
            // If the insertion-time g-score does not match the optimal g-score, ignore it
            if ((g_2_min.contains(curr_node) && !(inserted_g_score[1] < g_2_min.at(curr_node))) || (g_2_min_goal_set && !(inserted_f_score[1] < g_2_min_goal))) continue;

            g_2_min[curr_node] = inserted_g_score[2];

            // If current node satisfies goal condition, extract path and terminate
            if (problem.goal(curr_node)) {
                result.solution_set.emplace_back(std::vector<Node>(), std::vector<EDGE_STORAGE_T>(), inserted_g_score);
                extractPath(curr_enum_node, result.solution_set.back(), path_enum_node_map);
                //ASSERT(result.node_path.front() == problem.initial_node, "Extracted path initial node does not equal problem's initial node");
                //result.package();
                //return result;
                if (!g_2_min_goal_set || inserted_g_score[1] < g_2_min_goal) {
                    g_2_min_goal_set = true;
                    g_2_min_goal = inserted_g_score[1];
                }
                continue;
            }
            
            // If neighbors() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighbors)(SEARCH_PROBLEM_T, NODE_T)>::type neighbors = problem.neighbors(curr_node);
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, NODE_T)>::type to_neighbor_edges = problem.neighborEdges(curr_node);
            ASSERT(neighbors.size() == to_neighbor_edges.size(), "Number of neighbors does not match the number of outgoing edges");

            for (uint32_t i = 0; i < neighbors.size(); ++i) {
                Node neighbor = neighbors[i];
                
                // Extract pointer if the edge storage type is a const pointer
                EDGE_STORAGE_T to_neighbor_edge = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return &to_neighbor_edges[i];
                    else
                        return to_neighbor_edges[i];
                }();

                CV_T tentative_g_score = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return problem.gScore(inserted_g_score, *to_neighbor_edge);
                    else
                        return problem.gScore(inserted_g_score, to_neighbor_edge);
                }();

                CV_T tentative_h_score = tentative_g_score + problem.hScore(neighbor);

                // Prune nodes
                if ((g_2_min.contains(neighbor) && !(tentative_g_score[1] < g_2_min.at(neighbor))) || (g_2_min_goal_set && !(inserted_f_score[1] < g_2_min_goal))) continue;

                EnumeratedNode neighbor_enum_node = path_enum_node_map.newNode(neighbor, curr_enum_node);
                open_set.emplace(neighbor_enum_node, tentative_g_score, tentative_h_score);

            }
        }
        
        // Return failure
        result.package();
        return result;
    };

    template <class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    void BOAStar<EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::extractPath(const EnumeratedNode& goal_node, PathSolution<Node, EDGE_STORAGE_T, COST_T>& path_solution, const PathEnumeratedNodeMap<EDGE_STORAGE_T>& node_map) {
        EnumeratedNode curr_enum_node = goal_node;
        path_solution.node_path.emplace_back(node_map.getNode(curr_enum_node));

        while (node_map.hasParent(curr_enum_node)) {
            curr_enum_node = node_map.getParentEnumeratedNode(curr_enum_node);

            path_solution.node_path.emplace_back(node_map.getNode(curr_enum_node));
            path_solution.edge_path.emplace_back(node_map.getParentEdge(curr_enum_node));
        }
        std::reverse(path_solution.node_path.begin(), path_solution.node_path.end());
        std::reverse(path_solution.edge_path.begin(), path_solution.edge_path.end());
    }
    //void AStar<NODE_T, EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::extractPath(const NODE_T& goal_node, SingleObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T>& result) {

    //    result.success = true;
    //    result.path_cost = (*result.min_cost_map)[goal_node];

    //    NODE_T curr_node = goal_node;
    //    result.node_path.push_back(curr_node);

    //    auto found_it = result.search_tree->find(curr_node);
    //    while (found_it != result.search_tree->end()) { // While the key is contained
    //        const auto&[curr_node, edge] = found_it->second;

    //        result.node_path.emplace_back(curr_node);
    //        result.edge_path.emplace_back(edge);

    //        found_it = result.search_tree->find(curr_node);
    //    }
    //    std::reverse(result.node_path.begin(), result.node_path.end());
    //    std::reverse(result.edge_path.begin(), result.edge_path.end());
    //}
}
}