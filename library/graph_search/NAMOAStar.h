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




namespace TP {

// COST_T currently only requires operator<
template <typename T>
T abs(const T& x) {return (x < T{}) ? -x : x;}

namespace GraphSearch {



    template <ObjectiveCount M, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, Containers::FixedArray<M, COST_T>>, typename EDGE_STORAGE_T = EDGE_T>
    class NAMOAStar {
        public:
            static MultiObjectiveSearchResult<M, EDGE_STORAGE_T, COST_T> search(const SEARCH_PROBLEM_T& problem);
        private:
            //static void extractPath(const NODE_T& goal_node, MultiObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T>& result);
    };

    template <ObjectiveCount M, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    MultiObjectiveSearchResult<M, EDGE_STORAGE_T, COST_T> NAMOAStar<M, EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::search(const SEARCH_PROBLEM_T& problem) {
        using CV_T = Containers::FixedArray<M, COST_T>;

        // If custom edge storage type is used with explicit search, assert that the outgoingEdges method is explicit (returns a persistent const reference)
        constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, EDGE_T>::value;
        if constexpr (_PTR_EDGE_STORAGE_TYPE) {
            using _EDGE_CONTAINER_T = std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, NODE_T)>::type;
            constexpr bool _IS_EXPLICIT = std::is_reference<_EDGE_CONTAINER_T>();
            static_assert(_IS_EXPLICIT, "Must use explicit graph construction with non-default EDGE_STORAGE_T");
            static_assert(std::is_same<EDGE_STORAGE_T, const EDGE_T*>::value, "EDGE_STORAGE_T must be a persistent const COST_T pointer");
        }

        // Instantiate return value (search graph and non-dominated cost map are allocated)
        MultiObjectiveSearchResult<M, EDGE_STORAGE_T, COST_T> result;

        // Open set
        struct OpenSetElement {
            OpenSetElement(const Node& node_, const CV_T& g_score_, const CV_T& f_score_) : node(node_), g_score(g_score_), f_score(f_score_) {}
            Node node; 
            CV_T g_score; // g_score at the time of insertion ()
            CV_T f_score; // f_score (g_score + h)
        };
        auto less = [](const OpenSetElement& lhs, const OpenSetElement& rhs) -> bool {return rhs.f_score < lhs.f_score;}; // Open set element comparator (lhs and rhs are swapped for increasing order)
        std::priority_queue<OpenSetElement, std::vector<OpenSetElement>, decltype(less)> open_set;

        // Parent map
        SearchGraph<EDGE_STORAGE_T>& parent_graph = *(result.search_graph);

        // G-score container
        NonDominatedCostMap<COST_T>& G_set = *(result.non_dominated_cost_map);
        for (const auto& init_node : problem.initial_node_set) 
            g_score[init_node].addToOpen(CV_T{});

        // Add initial node to open set
        open_set.emplace(problem.initial_node, CV_T{}, problem.hScore(problem.initial_node));

        while (!open_set.empty()) {
            auto[curr_node, inserted_g_score, _] = open_set.top();
            open_set.pop(); 
            
            // If the insertion-time g-score does not match the optimal g-score, ignore it
            const COST_T& curr_node_g_score = g_score.at(curr_node);
            if (curr_node_g_score < inserted_g_score) continue;
            ASSERT(inserted_g_score == curr_node_g_score, "Inserted g-score is less than the min_cost_map g-score");

            // If current node satisfies goal condition, extract path and terminate
            if (problem.goal(curr_node)) {
                extractPath(curr_node, result);
                ASSERT(result.node_path.front() == problem.initial_node, "Extracted path initial node does not equal problem's initial node");
                result.package();
                return result;
            }
            
            // If neighbors() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighbors)(SEARCH_PROBLEM_T, NODE_T)>::type neighbors = problem.neighbors(curr_node);
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, NODE_T)>::type to_neighbor_edges = problem.neighborEdges(curr_node);
            ASSERT(neighbors.size() == to_neighbor_edges.size(), "Number of neighbors does not match the number of outgoing edges");

            for (uint32_t i = 0; i < neighbors.size(); ++i) {
                const NODE_T& neighbor = neighbors[i];
                
                // Extract pointer if the edge storage type is a const pointer
                EDGE_STORAGE_T to_neighbor_edge = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return &to_neighbor_edges[i];
                    else
                        return to_neighbor_edges[i];
                }();

                COST_T tentative_g_score = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return problem.gScore(curr_node_g_score, *to_neighbor_edge);
                    else
                        return problem.gScore(curr_node_g_score, to_neighbor_edge);
                }();

                // If 'neighbor' is not found in the min_cost_map, it's value is interpreted as 'infinity'
                auto it = g_score.find(neighbor);
                if (it == g_score.end() || tentative_g_score < it->second) {
                    parent_map.insert_or_assign(neighbor, Connection<NODE_T, EDGE_STORAGE_T>(curr_node, to_neighbor_edge));
                    g_score[neighbor] = tentative_g_score;
                    
                    COST_T f_score = tentative_g_score + problem.hScore(neighbor);
                    open_set.push(OpenSetElement{neighbor, tentative_g_score, f_score});
                }
            }
        }
        
        // Return failure
        result.package();
        return result;
    };

    //template <class NODE_T, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
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