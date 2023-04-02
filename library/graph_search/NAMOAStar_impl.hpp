#pragma once

#include "NAMOAStar.h"

#include <array>
#include <queue>
#include <map>
#include <memory>
#include <algorithm>
#include <type_traits>

#include "core/Graph.h"

#include "tools/Debug.h"
#include "tools/Logging.h"

#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"



namespace TP {


namespace GraphSearch {

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    NAMOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::NAMOASearchResult NAMOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::search(const SEARCH_PROBLEM_T& problem) {

        // If custom edge storage type is used with explicit search, assert that the outgoingEdges method is explicit (returns a persistent const reference)
        constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, edge_t>::value;
        if constexpr (_PTR_EDGE_STORAGE_TYPE) {
            using _EDGE_CONTAINER_T = std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, GraphNode)>::type;
            constexpr bool _IS_EXPLICIT = std::is_reference<_EDGE_CONTAINER_T>();
            static_assert(_IS_EXPLICIT, "Must use explicit graph construction with non-default EDGE_STORAGE_T");
            static_assert(std::is_same<EDGE_STORAGE_T, const edge_t*>::value, "EDGE_STORAGE_T must be a persistent const COST_T pointer");
        }

        // Instantiate return value (search graph and non-dominated cost map are allocated)
        NAMOASearchResult result(true, true);

        // Open set

        OpenSet open_set;

        // Parent map is represented as an acyclic graph
        SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, GraphNode>& parent_graph = *(result.search_graph);

        // G-score container
        CostMap& G_set = *(result.non_dominated_cost_map);

        for (const auto& init_node : problem.initial_node_set) {
            const CostMapItem* g_score_item = G_set.cost_map[init_node].addToOpen(COST_VECTOR_T{});

            // Add initial node to open set
            open_set.insert(init_node, g_score_item, problem.hScore(init_node));
        }

        std::vector<std::pair<GraphNode, const CostMapItem*>> solution_set;

        //
        auto edgeToStr = [](const SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>& edge) {
            return std::to_string(edge.cv->template get<0>()) + ", " + std::to_string(edge.cv->template get<1>());
        };
        auto costToStr = [](const COST_VECTOR_T& cost) {
            return std::to_string(cost.template get<0>()) + ", " + std::to_string(cost.template get<1>());
        };
        //

        while (!open_set.empty()) {

            // Retrieves a non dominated node, and removes it from the open set
            auto[curr_node, curr_g_score] = open_set.pop();

            //NEW_LINE;
            //LOG("Curr node: " << curr_node);
            //parent_graph.printCustom(edgeToStr);
            //PAUSE;
            
            // Move to closed
            CostMap::CostSet::moveToClosed(curr_g_score);
            
            
            // If current node satisfies goal condition, extract path and terminate
            if (problem.goal(curr_node)) {
                //LOG("-> goal");
                //LOG("-> path cost: "<< curr_g_score->cv.template get<0>() << ", " << curr_g_score->cv.template get<1>());

                // Add solution to the goal set
                solution_set.emplace_back(curr_node, curr_g_score);

                // Erase all dominated paths from open set
                //LOG("b4 erase dominated");
                open_set.eraseDominated(curr_g_score->cv);
                //LOG("af erase dominated");
                result.success = true;
                continue;
            }
            
            // If neighbors() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighbors)(SEARCH_PROBLEM_T, GraphNode)>::type neighbors = problem.neighbors(curr_node);
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, GraphNode)>::type to_neighbor_edges = problem.neighborEdges(curr_node);
            ASSERT(neighbors.size() == to_neighbor_edges.size(), "Number of neighbors does not match the number of outgoing edges");

            for (uint32_t i = 0; i < neighbors.size(); ++i) {

                GraphNode neighbor = neighbors[i];


                // Extract pointer if the edge storage type is a const pointer
                EDGE_STORAGE_T to_neighbor_edge = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return &to_neighbor_edges[i];
                    else
                        return to_neighbor_edges[i];
                }();

                COST_VECTOR_T tentative_g_score = [&] {
                    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                        return problem.gScore(curr_node, curr_g_score->cv, *to_neighbor_edge);
                    else
                        return problem.gScore(curr_node, curr_g_score->cv, to_neighbor_edge);
                }();

                //LOG("- neighbor: " << neighbor << " tentative cost: " << costToStr(tentative_g_score));

                auto it = G_set.cost_map.find(neighbor);
                if (it != G_set.cost_map.end() && G_set.cost_map.at(neighbor).dominates(tentative_g_score) == Containers::ArrayComparison::DoesNotDominate) {
                    // Signal when element is pruned
                    auto onErase = [&](const CostMapItem& item) {
                        //LOG("    erase domainated item ptr: " << &item << " in open: " << item.in_open);

                        // Remove edges from the search graph
                        auto disconnectEdge = [&item](GraphNode dst, const SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>& edge) {
                            //LOG("    erasing sg connection to dst" << dst);
                            return edge.cv == &item.cv; 
                        };
                        parent_graph.rdisconnectIf(neighbor, disconnectEdge);

                        // Remove the corresponding element from open set
                        if (item.in_open) {
                            open_set.erase(neighbor, &item);
                        }
                    };

                    // Erase all dominated paths in the cost map
                    G_set.cost_map.at(neighbor).eraseDominated(tentative_g_score, onErase);


                } else if (it != G_set.cost_map.end()) {
                    // If 'neighbor' is not found in the min_cost_map, it's value is interpreted as 'infinity' (new node)
                    //LOG("  -> found dominated path, skipping");
                    continue;
                }
                    
                //LOG("  -> adding " << neighbor << " to q and sg with cost: " << tentative_g_score.template get<0>() << ", " << tentative_g_score.template get<1>());
                // Calculate h score
                COST_VECTOR_T tentative_h_score = tentative_g_score;
                tentative_h_score += problem.hScore(neighbor);

                // Filter by the costs of goal nodes
                for (const auto& sol : solution_set) {
                    if (sol.second->cv.dominates(tentative_h_score) == Containers::ArrayComparison::Dominates) continue;
                }

                // Add the g_score to the cost map
                const CostMapItem* g_score_item = G_set.cost_map[neighbor].addToOpen(std::move(tentative_g_score));

                //LOG(" -> inserting into open set");
                open_set.insert(neighbor, g_score_item, std::move(tentative_h_score));

                //PAUSE_IF(neighbor == 30, "neighbor is 30! curr_node: " << curr_node << 
                //    " curr_g_score: " << curr_g_score->cv.template get<0>() << ", " << curr_g_score->cv.template get<1>() <<
                //    " new gscore: " << g_score_item->cv.template get<0>() << ", " << g_score_item->cv.template get<1>()
                //);
                parent_graph.connect(curr_node, neighbor, SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>(&(g_score_item->cv), std::move(to_neighbor_edge)));
                    
            }
        }
        
        // Extract the paths into the search result
        if (result.success) extractPaths(solution_set, result, problem);

        result.package();
        return result;
    };

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    void NAMOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::extractPaths(std::vector<std::pair<GraphNode, const CostMapItem*>>& solution_set, NAMOASearchResult& result, const SEARCH_PROBLEM_T& problem) {

        constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, edge_t>::value;

        // Copy the search graph so that edges can be removed
        SearchGraph<SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>, GraphNode> graph_copy = *result.search_graph;

        //
        //LOG("PRINTING FINAL SEARCH GRAPH");
        //auto edgeToStr = [](const SearchGraphEdge<const COST_VECTOR_T*, EDGE_STORAGE_T>& edge) {
        //    return std::to_string(edge.cv->template get<0>()) + ", " + std::to_string(edge.cv->template get<1>());
        //};
        //auto costToStr = [](const COST_VECTOR_T& cost) {
        //    return std::to_string(cost.template get<0>()) + ", " + std::to_string(cost.template get<1>());
        //};
        //graph_copy.printCustom(edgeToStr);
        //PAUSE;
        //

        // Copy and erase elements when the solutions are found
        std::vector<std::pair<GraphNode, const CostMapItem*>> solution_set_copy = solution_set;

        // Convert to search tree
        PathEnumeratedNodeMap<GraphNode, EnumeratedNode, SearchGraphEdge<COST_VECTOR_T, EDGE_STORAGE_T>> tree;

        std::vector<std::pair<GraphNode, COST_VECTOR_T>> stack;
        std::map<GraphNode, bool> visited;
        //stack.reserve(problem.inital_node_set.size());
        for (const auto& init_node : problem.initial_node_set) {
            auto en_node = tree.newInitNode(init_node);
            stack.emplace_back(en_node, COST_VECTOR_T{});
        }

        while (!stack.empty()) {
            auto[curr_enum_node, curr_cost] = stack.back();
            GraphNode curr_node = tree.getNode(curr_enum_node);
            //LOG("curr node: " << curr_node);
            //visited[curr_node] = true;
            //PAUSE;

            stack.pop_back();

            if (problem.goal(curr_node)) {
                for (auto sol_it = solution_set_copy.begin(); sol_it != solution_set_copy.end();) {
                    if (curr_cost == sol_it->second->cv) {
                        result.solution_set.push_back(PathSolution<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>());
                        auto& path_solution = result.solution_set.back();
                        path_solution.path_cost = std::move(curr_cost);
                        extractPath(curr_enum_node, path_solution, tree);

                        solution_set_copy.erase(sol_it);
                        break;
                    }
                    ++sol_it;
                }
                if (solution_set_copy.empty()) return;
                continue;
            }
            
            const auto& children = graph_copy.getChildren(curr_node);
            const auto& outgoing_edges = graph_copy.getOutgoingEdges(curr_node);
            for (uint32_t i=0; i<children.size(); ++i) {
                //LOG("   child: " << children[i]);
                //LOG("   edge: " << edgeToStr(outgoing_edges[i]));
                //if (visited.contains(children[i])) continue;

                COST_VECTOR_T tentative_cost = *outgoing_edges[i].cv;

                if (!(curr_cost <= tentative_cost)) continue;
                //COST_VECTOR_T tentative_cost = [&] {
                //    if constexpr (_PTR_EDGE_STORAGE_TYPE)
                //        return problem.gScore(curr_cost, *(outgoing_edges[i].edge));
                //    else
                //        return problem.gScore(curr_cost, outgoing_edges[i].edge);
                //}();

                bool dominated = false;
                //LOG("       testing " << costToStr(tentative_cost));
                for (uint32_t j=0; j<solution_set.size(); ++j) {
                    auto path_test_result = solution_set[j].second->cv.dominates(tentative_cost);
                    if (path_test_result == Containers::ArrayComparison::Dominates) {
                        // A goal node dominates the path
                        //LOG("       Dominated by " << costToStr(solution_set[j].second->cv));
                        dominated = true;
                        break;
                    } 
                }
                //if (dominated) LOG("   dominated!");
                if (dominated) continue;

                //LOG("   adding...");
                stack.emplace_back(GraphNode{},tentative_cost);
                stack.back().first = tree.newNode(children[i], curr_enum_node, SearchGraphEdge<COST_VECTOR_T, EDGE_STORAGE_T>(std::move(tentative_cost), outgoing_edges[i].edge));
            }
        }
    }

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    void NAMOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::extractPath(const GraphNode& goal_node, PathSolution<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>& path_solution, const PathEnumeratedNodeMap<GraphNode, EnumeratedNode, SearchGraphEdge<COST_VECTOR_T, EDGE_STORAGE_T>>& node_map) {
        EnumeratedNode curr_enum_node = goal_node;
        path_solution.node_path.emplace_back(node_map.getNode(curr_enum_node));

        while (!node_map.isInit(curr_enum_node)) {
            // Push back edge
            path_solution.edge_path.emplace_back(node_map.getParentEdge(curr_enum_node).edge);

            // Set curr to the parent
            curr_enum_node = node_map.getParentEnumeratedNode(curr_enum_node);

            // Push back the parent
            path_solution.node_path.emplace_back(node_map.getNode(curr_enum_node));

        }
        std::reverse(path_solution.node_path.begin(), path_solution.node_path.end());
        std::reverse(path_solution.edge_path.begin(), path_solution.edge_path.end());

    }
}
}