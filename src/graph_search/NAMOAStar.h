#pragma once

#include <array>
#include <queue>
#include <map>
#include <memory>
#include <algorithm>
#include <type_traits>

#include "tools/Logging.h"
#include "graph_search/SearchProblem.h"
#include "core/Graph.h"


namespace TP {
namespace GraphSearch {

    using ObjectiveCount = uint8_t;

    template<class COST_T, ObjectiveCount M>
    struct CostVector {
        CostVector() = default;
        std::array<COST_T, M> values = std::array<COST_T, M>();
        bool operator==(const CostVector& other) const {return values == other.values;}
        bool operator<(const CostVector& other) const {
            bool equal = true;
            for (ObjectiveCount i=0; i < M; ++i) if (values[i] > other.values[i]) {
                return false;
            } else {
                if (equal && values[i] < other.values[i]) equal = false;
            }
            return equal;
        }
    };

    template <class EDGE_STORAGE_T>
    using SearchGraph = Graph<EDGE_STORAGE_T>;

    template <class COST_VECTOR_T>
    struct NonDominatedCostMap {
        public:
            class OrderedCostSet {
                public:
                    OrderedCostSet() = default;
                    inline void eraseDominated(const COST_VECTOR_T& cost_vector) {m_set.erase(m_set.lower_bound(cost_vector), m_set.end());}
                    inline void addToOpen(const COST_VECTOR_T& v) {m_set[v] = true;}
                    inline void addToClosed(const COST_VECTOR_T& v) {m_set[v] = false;}
                    inline void moveToClosed(const COST_VECTOR_T& v) {m_set[v] = false;}
                    inline bool contains(const COST_VECTOR_T& v) const {return m_set.contains(v);}
                private:
                    std::map<COST_VECTOR_T, bool> m_set;
            };

        public:
            std::map<Node, OrderedCostSet> cost_map;
    };
    
    template <class EDGE_STORAGE_T, class COST_T, ObjectiveCount M>
    struct MultiObjectiveSearchResult {
        public:
            struct Solution {
                std::vector<Node> node_path;
                std::vector<EDGE_STORAGE_T> edge_path;
                COST_T path_cost = COST_T{};
            };
        public:
            MultiObjectiveSearchResult(bool retain_search_graph = true, bool retain_non_dominated_cost_map = true)
                : search_tree(std::make_shared<SearchGraph<EDGE_STORAGE_T>>())
                , min_cost_map(std::make_shared<NonDominatedCostMap<NODE_T, CostVector<COST_T, M>>()) 
                , m_retain_search_graph(retain_search_graph)
                , m_retain_non_dominated_cost_map(retain_non_dominated_cost_map)
                {}

            std::vector<Solution> solution_set;
            uint32_t solutions_count = 0;
            std::shared_ptr<SearchGraph<EDGE_STORAGE_T>> search_graph;
            std::shared_ptr<NonDominatedCostMap<CostVector<COST_T, M>>> non_dominated_cost_map;

            void package() { // Free the memory of the search tree and min cost map if the user desires
                if (!m_retain_search_graph) search_tree.reset();
                if (!m_retain_non_dominated_cost_map) non_dominated_cost_map.reset();
            }
        private:
            bool m_retain_search_graph, m_retain_non_dominated_cost_map;
    };



    template <ObjectiveCount M, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, CostVector<COST_T, M>>, typename EDGE_STORAGE_T = EDGE_T>
    class NAMOAStar {
        public:
            static MultiObjectiveSearchResult<EDGE_STORAGE_T, COST_T, M> search(const SEARCH_PROBLEM_T& problem);
        private:
            static void extractPath(const NODE_T& goal_node, MultiObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T>& result);
    };

    //template <class NODE_T, class EDGE_T, class COST_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    //SingleObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T> AStar<NODE_T, EDGE_T, COST_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::search(const SEARCH_PROBLEM_T& problem) {


    //    // If custom edge storage type is used with explicit search, assert that the outgoingEdges method is explicit (returns a persistent const reference)
    //    constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, EDGE_T>::value;
    //    if constexpr (_PTR_EDGE_STORAGE_TYPE) {
    //        using _EDGE_CONTAINER_T = std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, NODE_T)>::type;
    //        constexpr bool _IS_EXPLICIT = std::is_reference<_EDGE_CONTAINER_T>();
    //        static_assert(_IS_EXPLICIT, "Must use explicit graph construction with non-default EDGE_STORAGE_T");
    //        static_assert(std::is_same<EDGE_STORAGE_T, const EDGE_T*>::value, "EDGE_STORAGE_T must be a persistent const COST_T pointer");
    //    }

    //    // Instantiate return value (success is initialized to false and search tree and min cost map are allocated)
    //    SingleObjectiveSearchResult<NODE_T, EDGE_STORAGE_T, COST_T> result;

    //    // Open set
    //    struct OpenSetElement {
    //        OpenSetElement(const NODE_T& node_, const COST_T& g_score_, const COST_T& f_score_) : node(node_), g_score(g_score_), f_score(f_score_) {}
    //        NODE_T node; 
    //        COST_T g_score; // g_score at the time of insertion ()
    //        COST_T f_score; // f_score (g_score + h)
    //    };
    //    auto less = [](const OpenSetElement& lhs, const OpenSetElement& rhs) -> bool {return lhs.f_score > rhs.f_score;}; // Open set element comparator
    //    std::priority_queue<OpenSetElement, std::vector<OpenSetElement>, decltype(less)> open_set;

    //    // Parent map
    //    SearchTree<NODE_T, EDGE_STORAGE_T>& parent_map = *(result.search_tree);

    //    // G-score container
    //    MinCostMap<NODE_T, COST_T>& g_score = *(result.min_cost_map);
    //    g_score[problem.initial_node] = COST_T{};

    //    // Add initial node to open set
    //    open_set.emplace(problem.initial_node, COST_T{}, problem.hScore(problem.initial_node));

    //    while (!open_set.empty()) {
    //        auto[curr_node, inserted_g_score, _] = open_set.top();
    //        open_set.pop(); 
    //        
    //        // If the insertion-time g-score does not match the optimal g-score, ignore it
    //        const COST_T& curr_node_g_score = g_score.at(curr_node);
    //        if (inserted_g_score > curr_node_g_score) continue;
    //        ASSERT(inserted_g_score == curr_node_g_score, "Inserted g-score is less than the min_cost_map g-score");

    //        // If current node satisfies goal condition, extract path and terminate
    //        if (problem.goal(curr_node)) {
    //            extractPath(curr_node, result);
    //            ASSERT(result.node_path.front() == problem.initial_node, "Extracted path initial node does not equal problem's initial node");
    //            result.package();
    //            return result;
    //        }
    //        
    //        // If neighbors() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
    //        typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighbors)(SEARCH_PROBLEM_T, NODE_T)>::type neighbors = problem.neighbors(curr_node);
    //        typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, NODE_T)>::type to_neighbor_edges = problem.neighborEdges(curr_node);
    //        ASSERT(neighbors.size() == to_neighbor_edges.size(), "Number of neighbors does not match the number of outgoing edges");

    //        for (uint32_t i = 0; i < neighbors.size(); ++i) {
    //            const NODE_T& neighbor = neighbors[i];
    //            
    //            // Extract pointer if the edge storage type is a const pointer
    //            EDGE_STORAGE_T to_neighbor_edge = [&] {
    //                if constexpr (_PTR_EDGE_STORAGE_TYPE)
    //                    return &to_neighbor_edges[i];
    //                else
    //                    return to_neighbor_edges[i];
    //            }();

    //            COST_T tentative_g_score = [&] {
    //                if constexpr (_PTR_EDGE_STORAGE_TYPE)
    //                    return problem.gScore(curr_node_g_score, *to_neighbor_edge);
    //                else
    //                    return problem.gScore(curr_node_g_score, to_neighbor_edge);
    //            }();

    //            // If 'neighbor' is not found in the min_cost_map, it's value is interpreted as 'infinity'
    //            auto it = g_score.find(neighbor);
    //            if (it == g_score.end() || tentative_g_score < it->second) {
    //                parent_map.insert_or_assign(neighbor, Connection<NODE_T, EDGE_STORAGE_T>(curr_node, to_neighbor_edge));
    //                g_score[neighbor] = tentative_g_score;
    //                
    //                COST_T f_score = tentative_g_score + problem.hScore(neighbor);
    //                open_set.push(OpenSetElement{neighbor, tentative_g_score, f_score});
    //            }
    //        }
    //    }
    //    
    //    // Return failure
    //    result.package();
    //    return result;
    //};

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