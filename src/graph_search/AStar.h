#pragma once

#include <queue>
#include <set>
#include <map>
#include <memory>
#include <algorithm>
#include <type_traits>

#include "tools/Logging.h"
#include "graph_search/SearchProblem.h"


namespace TP {
namespace GraphSearch {

    template <class NODE_T, class EDGE_T>
    struct Connection {
        Connection(const NODE_T& node_, const EDGE_T& edge_) : node(node_), edge(edge_) {}
        NODE_T node;
        EDGE_T edge;
    };

    template <class NODE_T, class EDGE_T>
    using SearchTree = std::map<NODE_T, Connection<NODE_T, EDGE_T>>;
    
    template <class NODE_T, class COST_T>
    using MinCostMap = std::map<NODE_T, COST_T>;

    template <class NODE_T, class EDGE_T, class COST_T>
    struct SingleObjectiveSearchResult {
        public:
            SingleObjectiveSearchResult(bool retain_search_tree = true, bool retain_min_cost_map = true)
                : search_tree(std::make_shared<SearchTree<NODE_T, EDGE_T>>())
                , min_cost_map(std::make_shared<MinCostMap<NODE_T, COST_T>>()) 
                , m_retain_search_tree(retain_search_tree)
                , m_retain_min_cost_map(retain_min_cost_map)
                {}

            bool success = false;
            std::vector<NODE_T> node_path;
            std::vector<EDGE_T> edge_path;
            COST_T path_cost = COST_T{};
            std::shared_ptr<SearchTree<NODE_T, EDGE_T>> search_tree;
            std::shared_ptr<MinCostMap<NODE_T, COST_T>> min_cost_map;

            void package() { // Free the memory of the search tree and min cost map if the user desires
                if (!m_retain_search_tree) search_tree.reset();
                if (!m_retain_min_cost_map) min_cost_map.reset();
            }
        private:
            bool m_retain_search_tree, m_retain_min_cost_map;
    };

    template <class NODE_T, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<NODE_T, COST_T>>
    class AStar {
        public:
            static SingleObjectiveSearchResult<NODE_T, EDGE_T, COST_T> search(const SEARCH_PROBLEM_T& problem);
        private:
            static void extractPath(const NODE_T& goal_node, SingleObjectiveSearchResult<NODE_T, EDGE_T, COST_T>& result);
    };

    template <class NODE_T, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T>
    SingleObjectiveSearchResult<NODE_T, EDGE_T, COST_T> AStar<NODE_T, EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T>::search(const SEARCH_PROBLEM_T& problem) {
        // Instantiate return value (success is initialized to false and search tree and min cost map are allocated)
        SingleObjectiveSearchResult<NODE_T, EDGE_T, COST_T> result;

        // Open set
        using OpenSetElement = std::pair<NODE_T, COST_T>; // Open set element type (node, f-score)
        auto less = [](const OpenSetElement& lhs, const OpenSetElement& rhs) -> bool {return lhs.second < rhs.second;}; // Open set element comparator
        std::priority_queue<OpenSetElement, std::vector<OpenSetElement>, decltype(less)> open_set;
        std::set<NODE_T> open_set_unique;

        // Parent map
        SearchTree<NODE_T, EDGE_T>& parent_map = *(result.search_tree);

        // G-score container
        MinCostMap<NODE_T, COST_T>& g_score = *(result.min_cost_map);
        g_score[problem.initial_node] = COST_T{};

        // Add initial node to open set
        open_set.emplace(problem.initial_node, problem.hScore(problem.initial_node));
        open_set_unique.insert(problem.initial_node);

        while (!open_set.empty()) {
            NODE_T curr_node = open_set.top().first;
            open_set.pop(); // remove from queue
            open_set_unique.erase(curr_node); // remove from uniqueness set

            // If current node satisfies goal condition, extract path and terminate
            if (problem.goal(curr_node)) {
                extractPath(curr_node, result);
                ASSERT(result.node_path.front() == problem.initial_node, "Extracted path initial node does not equal problem's initial node");
                result.package();
                return result;
            }
            
            // If children() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::children)(SEARCH_PROBLEM_T, NODE_T)>::type children = problem.children(curr_node);
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::outgoingEdges)(SEARCH_PROBLEM_T, NODE_T)>::type outgoing_edges = problem.outgoingEdges(curr_node);
            ASSERT(children.size() == outgoing_edges.size(), "Number of children does not match the number of outgoing edges");

            for (uint32_t i = 0; i < children.size(); ++i) {
                const NODE_T& child = children[i];
                const EDGE_T& to_child_edge = outgoing_edges[i];

                COST_T tentative_g_score = problem.gScore(g_score.at(curr_node), to_child_edge);

                // If 'child' is not found in the min_cost_map, it's value is interpreted as 'infinity'
                auto it = g_score.find(child);
                if (it == g_score.end() || tentative_g_score < it->second) {
                    //auto pari = std::make_pair(curr_node, to_child_edge);
                    //parent_map[child] = Connection<NODE_T, EDGE_T>{curr_node, to_child_edge};
                    parent_map.try_emplace(child, curr_node, to_child_edge);
                    g_score[child] = tentative_g_score;
                    
                    if (!open_set_unique.contains(child)) {
                        COST_T f_score = tentative_g_score + problem.hScore(child);
                        open_set.push(OpenSetElement{child, f_score});
                        open_set_unique.insert(child);
                    }
                }
            }
        }
        
        // Return failure
        result.package();
        return result;
    };

    template <class NODE_T, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T>
    void AStar<NODE_T, EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T>::extractPath(const NODE_T& goal_node, SingleObjectiveSearchResult<NODE_T, EDGE_T, COST_T>& result) {

        result.success = true;
        result.path_cost = (*result.min_cost_map)[goal_node];

        NODE_T curr_node = goal_node;
        result.node_path.push_back(curr_node);

        auto found_it = result.search_tree->find(curr_node);
        while (found_it != result.search_tree->end()) { // While the key is contained
            const auto&[curr_node, edge] = found_it->second;

            result.node_path.push_back(curr_node);
            result.edge_path.push_back(edge);

            found_it = result.search_tree->find(curr_node);
        }
        std::reverse(result.node_path.begin(), result.node_path.end());
        std::reverse(result.edge_path.begin(), result.edge_path.end());
    }
}
}