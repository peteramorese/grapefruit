#pragma once

#include "NAMOAStar.h"

#include <array>
#include <queue>
#include <map>
#include <memory>
#include <algorithm>
#include <type_traits>

#include "tools/Logging.h"
#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "core/Graph.h"




namespace TP {


namespace GraphSearch {

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T, class OBJECTIVE_NORM_T>
    MultiObjectiveSearchResult<typename SEARCH_PROBLEM_T::node_t, EDGE_STORAGE_T, COST_VECTOR_T> NAMOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T, OBJECTIVE_NORM_T>::search(const SEARCH_PROBLEM_T& problem) {

        // If custom edge storage type is used with explicit search, assert that the outgoingEdges method is explicit (returns a persistent const reference)
        constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, edge_t>::value;
        if constexpr (_PTR_EDGE_STORAGE_TYPE) {
            using _EDGE_CONTAINER_T = std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, GraphNode)>::type;
            constexpr bool _IS_EXPLICIT = std::is_reference<_EDGE_CONTAINER_T>();
            static_assert(_IS_EXPLICIT, "Must use explicit graph construction with non-default EDGE_STORAGE_T");
            static_assert(std::is_same<EDGE_STORAGE_T, const edge_t*>::value, "EDGE_STORAGE_T must be a persistent const COST_T pointer");
        }

        // Instantiate return value (search graph and non-dominated cost map are allocated)
        MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> result(true, true);

        // Open set
        struct OpenSetElement {
            OpenSetElement() = delete;
            
            // Used for constructing a key to remove
            OpenSetElement(const GraphNode& node_, const CostMapItem* g_score_) 
                : g_score_entry(GSetEntry(node_, g_score_)) {}

            OpenSetElement(const GraphNode& node_, const CostMapItem* g_score_, COST_VECTOR_T&& f_score_) 
                : g_score_entry(GSetEntry(node_, g_score_))
                , f_score(f_score_) 
                , cached_norm(OBJECTIVE_NORM_T::norm(f_score)) {}

            bool operator==(const OpenSetElement& other) const {return g_score_entry == other.g_score_entry;}
            bool operator<(const OpenSetElement& other) const {return cached_norm < other.cached_norm;}

            GSetEntry g_score_entry;
            COST_VECTOR_T f_score; // f_score (g_score + h)
            OBJECTIVE_NORM_T::combined_cost_t cached_norm; // Cache the norm calculation

        };
        std::set<OpenSetElement> open_set;

        // Parent map is represented as an acyclic graph
        SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, GraphNode>& parent_graph = *(result.search_graph);

        // G-score container
        NonDominatedCostMap<COST_VECTOR_T>& G_set = *(result.non_dominated_cost_map);

        for (const auto& init_node : problem.initial_node_set) {
            const CostMapItem* g_score_item = G_set.cost_map[init_node].addToOpen(COST_VECTOR_T{});

            // Add initial node to open set
            open_set.emplace(init_node, g_score_item, problem.hScore(init_node));
        }

        std::vector<GSetEntry> goal_set;

        while (!open_set.empty()) {
            auto& curr_entry = open_set.begin()->g_score_entry;
            GraphNode curr_node = curr_entry.node;
            const CostMapItem* curr_g_score = curr_entry.g_score;
            
            // Move to closed
            NonDominatedCostMap<COST_VECTOR_T>::CostSet::moveToClosed(curr_g_score);
            
            // Remove the element from the open set
            open_set.erase(open_set.begin()); 
            
            // If current node satisfies goal condition, extract path and terminate
            if (problem.goal(curr_node)) {
                goal_set.emplace_back(curr_node, curr_g_score);
                for (auto it = open_set.begin(); it != open_set.end();) {
                    if (curr_g_score->cv.dominates(it->g_score_entry.g_score->cv) == Containers::ArrayComparison::Dominates) {
                        open_set.erase(it++);
                    } else {
                        ++it;
                    }
                }
                result.success = true;
                continue;
            }
            
            // If neighbors() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighbors)(SEARCH_PROBLEM_T, GraphNode)>::type neighbors = problem.neighbors(curr_node);
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, GraphNode)>::type to_neighbor_edges = problem.neighborEdges(curr_node);
            ASSERT(neighbors.size() == to_neighbor_edges.size(), "Number of neighbors does not match the number of outgoing edges");

            for (uint32_t i = 0; i < neighbors.size(); ++i) {

                GraphNode neighbor = neighbors[i];

                // Check if a cycle is produced
                if (!parent_graph.testConnection(curr_node, neighbor)) continue;
                
                // Extract pointer if the edge storage type is a const pointer
                EDGE_STORAGE_T to_neighbor_edge = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return &to_neighbor_edges[i];
                    else
                        return to_neighbor_edges[i];
                }();

                COST_VECTOR_T tentative_g_score = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return problem.gScore(curr_g_score->cv, *to_neighbor_edge);
                    else
                        return problem.gScore(curr_g_score->cv, to_neighbor_edge);
                }();

                auto it = G_set.cost_map.find(neighbor);
                if (it != G_set.cost_map.end() && G_set.cost_map.at(neighbor).dominates(tentative_g_score) == Containers::ArrayComparison::DoesNotDominate) {

                    // Signal when element is pruned
                    auto onErase = [&](const CostMapItem& item) {
                        // Remove edges from the search graph
                        auto disconnectEdge = [&item](GraphNode dst, const SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>& edge) {
                            return edge.cv == &item.cv; 
                        };
                        parent_graph.disconnectIf(neighbor, disconnectEdge);

                        // Remove the corresponding element from open set
                        if (item.in_open) open_set.erase(OpenSetElement(neighbor, &item));
                    };

                    // Erase all dominated paths in the cost map
                    G_set.cost_map.at(neighbor).eraseDominated(tentative_g_score, onErase);

                } else if (it != G_set.cost_map.end()) {
                    // If 'neighbor' is not found in the min_cost_map, it's value is interpreted as 'infinity' (new node)
                    continue;
                }
                    
                // Calculate h score
                COST_VECTOR_T tentative_h_score = tentative_g_score;
                tentative_h_score += problem.hScore(neighbor);

                // Filter by the costs of goal nodes
                for (const auto& goal_g_set_entry : goal_set) {
                    if (goal_g_set_entry.g_score->cv.dominates(tentative_h_score) == Containers::ArrayComparison::Dominates) continue;
                }

                // Add the g_score to the cost map
                const CostMapItem* g_score_item = G_set.cost_map[neighbor].addToOpen(std::move(tentative_g_score));

                open_set.emplace(neighbor, g_score_item, std::move(tentative_h_score));

                parent_graph.connect(curr_node, neighbor, SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>(&(g_score_item->cv), std::move(to_neighbor_edge)));
                    
            }
        }
        
        // Return failure
        result.package();
        return result;
    };

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T, class OBJECTIVE_NORM_T>
    void NAMOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T, OBJECTIVE_NORM_T>::extractPaths(std::vector<GSetEntry> goal_set, MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>& result, const SEARCH_PROBLEM_T& problem) {

        // Copy the search graph so that edges can be removed
        SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, GraphNode> graph_copy = *result.search_graph;

        // While there are remaining goal nodes
        while (!goal_set.empty()) {
            auto[g_score, goal_node] = goal_set.back();
            goal_set.pop_back();

            // Assign the solution cost
            std::vector<GraphNode> node_path;
            std::vector<EDGE_STORAGE_T> edge_path;

            // Search graph is reversed, so traverse backwards
            GraphNode parent = goal_node;
            GraphNode child = goal_node;
            const std::vector<GraphNode>* children = &graph_copy.getChildren(parent);
            while (!children->empty()) {
                child = (*children)[0];
                node_path.push_back(child);
                edge_path.push_back(graph_copy.getChildren(parent)[0].edge);

                if (children.size() > 1) { // More than one child, branch exists, need to prune
                    graph_copy.disconnect(parent, child);
                }

                // If the child is a leaf, terminate
                if (graph_copy.getChildren(child).empty()) break;

                // Otherwise set parent to child and continue
                parent = child;
                children = &graph_copy.getChildren(parent);
            } 

            if (problem.initial_node_set.contains(child)) {
                std::reverse(node_path.begin(), node_path.end());
                std::reverse(edge_path.begin(), edge_path.end());
                result.solution_set.emplace_back(std::move(node_path), std::move(edge_path), g_score->cv);
            } else {
                graph_copy.dis
            }
        }
    }

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
    //}
}
}