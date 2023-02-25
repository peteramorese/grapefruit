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


    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, Containers::FixedArray<2, typename SEARCH_PROBLEM_T::cost_t>>, typename EDGE_STORAGE_T = typename SEARCH_PROBLEM_T::edge_t>
    class BOAStar {
        public:
            using GraphNode = SEARCH_PROBLEM_T::node_t;
            typedef SEARCH_PROBLEM_T::edge_t edge_t;
            typedef SEARCH_PROBLEM_T::cost_t cost_t;

        public:
            static MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> search(const SEARCH_PROBLEM_T& problem);

        private:
            using EnumeratedNode = GraphNode;
            
            class PathEnumeratedNodeMap {
                private:
                    struct Value {
                        Value() = delete;
                        Value(GraphNode node_, EnumeratedNode parent_, EDGE_STORAGE_T parent_edge_) : node(node_), parent(parent_), parent_edge(parent_edge_) {}
                        GraphNode node;
                        EnumeratedNode parent;
                        EDGE_STORAGE_T parent_edge;
                    };
                public:
                    inline bool isInit(EnumeratedNode enumerated_node) const {return m_init_nodes.contains(enumerated_node);}
                    inline GraphNode getNode(EnumeratedNode enumerated_node) const {
                        return (isInit(enumerated_node)) ? m_init_nodes.at(enumerated_node) : m_map.at(enumerated_node).node;
                    }
                    inline EnumeratedNode getParentEnumeratedNode(EnumeratedNode enumerated_node) const {return m_map.at(enumerated_node).parent;}
                    inline const EDGE_STORAGE_T& getParentEdge(EnumeratedNode enumerated_node) const {return m_map.at(enumerated_node).parent_edge;}
                    inline EnumeratedNode newNode(GraphNode node, EnumeratedNode parent, const EDGE_STORAGE_T& parent_edge) {
                        m_map.try_emplace(m_next_node, node, parent, parent_edge);
                        return m_next_node++;
                    }
                    inline EnumeratedNode newInitNode(GraphNode init_node) {
                        m_init_nodes.try_emplace(m_next_node, init_node);
                        return m_next_node++;
                    }
                private:
                    EnumeratedNode m_next_node = EnumeratedNode{};
                    std::map<EnumeratedNode, Value> m_map; // Maps enum node key to the actual node and the parent enum node
                    std::map<EnumeratedNode, GraphNode> m_init_nodes;
            };

        private:
            static void extractPath(const EnumeratedNode& goal_node, PathSolution<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>& path_solution, const PathEnumeratedNodeMap& node_map);

            template <typename RETURN_T, typename... ARGS_T>
            RETURN_T returnVal(RETURN_T(ARGS_T...));
    };

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    MultiObjectiveSearchResult<typename SEARCH_PROBLEM_T::node_t, EDGE_STORAGE_T, COST_VECTOR_T> BOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::search(const SEARCH_PROBLEM_T& problem) {

        // If custom edge storage type is used with explicit search, assert that the outgoingEdges method is explicit (returns a persistent const reference)
        constexpr bool _PTR_EDGE_STORAGE_TYPE = !std::is_same<EDGE_STORAGE_T, edge_t>::value;
        if constexpr (_PTR_EDGE_STORAGE_TYPE) {
            using _EDGE_CONTAINER_T = std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, GraphNode)>::type;
            constexpr bool _IS_EXPLICIT = std::is_reference<_EDGE_CONTAINER_T>();
            static_assert(_IS_EXPLICIT, "Must use explicit graph construction with non-default EDGE_STORAGE_T");
            static_assert(std::is_same<EDGE_STORAGE_T, const edge_t*>::value, "EDGE_STORAGE_T must be a persistent const COST_T pointer");
        }
        static_assert(COST_VECTOR_T::size() == 2, "BOA can only use two objectives");

        LOG("b4 search");
        // Instantiate return value (search graph and non-dominated cost map are allocated, but not used for this algorithm)
        MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> result(false, false);

        // Open set
        struct OpenSetElement {
            OpenSetElement(const EnumeratedNode& node_, const COST_VECTOR_T& g_score_, const COST_VECTOR_T& f_score_) : node(node_), g_score(g_score_), f_score(f_score_) {}
            EnumeratedNode node; 
            COST_VECTOR_T g_score; // g_score at the time of insertion ()
            COST_VECTOR_T f_score; // f_score (g_score + h)
        };
        auto less = [](const OpenSetElement& lhs, const OpenSetElement& rhs) -> bool {return rhs.f_score.lexicographicLess(lhs.f_score);}; // Open set element lexicographic comparator (lhs and rhs are swapped for increasing order)
        std::priority_queue<OpenSetElement, std::vector<OpenSetElement>, decltype(less)> open_set;

        // Enum node map (maps 'nodes' AKA EnumeratedNode to 'states' AKA Node in BOA), includes parent map
        PathEnumeratedNodeMap path_enum_node_map;

        // G-score container (g_2 min cost map for second objective)
        auto deduce_val = (COST_VECTOR_T{}).template get<1>();
        using cost_2_t = decltype(deduce_val); //std::result_of<decltype(&COST_VECTOR_T::get<2>)(COST_VECTOR_T)>::type;
        //using cost_2_t = decltype(deduce_val.template get<1>()); //std::result_of<decltype(&COST_VECTOR_T::get<2>)(COST_VECTOR_T)>::type;
        MinCostMap<GraphNode, cost_2_t> g_2_min;
        for (const auto& init_node : problem.initial_node_set) {
            EnumeratedNode init_enum_node = path_enum_node_map.newInitNode(init_node);

            // Add initial node to open set
            open_set.emplace(init_enum_node, COST_VECTOR_T{}, problem.hScore(init_node));
        }


        // Keep track of the min cost to any goal node
        bool g_2_min_goal_set = false;
        cost_2_t g_2_min_goal = cost_2_t{};


        while (!open_set.empty()) {
            LOG("in while");
            auto[curr_enum_node, inserted_g_score, inserted_f_score] = open_set.top();
            GraphNode curr_node = path_enum_node_map.getNode(curr_enum_node);
            open_set.pop(); 
            LOG("af 1");
            
            // If the insertion-time g-score does not match the optimal g-score, ignore it
            bool g_2_min_contains = g_2_min.contains(curr_node);
            g_2_min.find(curr_node)->second;
            if ((g_2_min_contains && !((inserted_g_score.template get<1>()) < g_2_min.find(curr_node)->second)) || (g_2_min_goal_set && !((inserted_f_score.template get<1>()) < g_2_min_goal))) continue;

            LOG("af 2");
            if (g_2_min_contains) {
                g_2_min.find(curr_node)->second = inserted_g_score.template get<1>();
            } else {
                g_2_min.try_emplace(curr_node, inserted_g_score.template get<1>());
            }
            //g_2_min[curr_node] = inserted_g_score[1];

            // If current node satisfies goal condition, extract path and terminate
            if (problem.goal(curr_node)) {
                result.success = true;
                PathSolution<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T> sol;
                sol.path_cost = inserted_g_score;
                extractPath(curr_enum_node, sol, path_enum_node_map);
                result.solution_set.push_back(std::move(sol));

                // Update the g_2_min across all goal nodes
                if (!g_2_min_goal_set || inserted_g_score.template get<1>() < g_2_min_goal) {
                    g_2_min_goal_set = true;
                    g_2_min_goal = inserted_g_score.template get<1>();
                }
                continue;
            }
            LOG("af 3");
            
            // If neighbors() and outgoingEdges() return a persistent const reference, do not copy, otherwise do copy
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighbors)(SEARCH_PROBLEM_T, GraphNode)>::type neighbors = problem.neighbors(curr_node);
            LOG("af 4");
            typename std::result_of<decltype(&SEARCH_PROBLEM_T::neighborEdges)(SEARCH_PROBLEM_T, GraphNode)>::type to_neighbor_edges = problem.neighborEdges(curr_node);
            LOG("af 5");
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
                        return problem.gScore(inserted_g_score, *to_neighbor_edge);
                    else
                        return problem.gScore(inserted_g_score, to_neighbor_edge);
                }();

                COST_VECTOR_T tentative_h_score = tentative_g_score;
                tentative_h_score += problem.hScore(neighbor);

                // Prune nodes
                if ((g_2_min.contains(neighbor) && !(tentative_g_score.template get<1>() < g_2_min.find(neighbor)->second)) || (g_2_min_goal_set && !(inserted_f_score.template get<1>() < g_2_min_goal))) continue;

                EnumeratedNode neighbor_enum_node = path_enum_node_map.newNode(neighbor, curr_enum_node, to_neighbor_edge);
                open_set.emplace(neighbor_enum_node, tentative_g_score, tentative_h_score);

            }
        }
        
        // Return failure
        result.package();
        return result;
    };

    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T, typename EDGE_STORAGE_T>
    void BOAStar<COST_VECTOR_T, SEARCH_PROBLEM_T, HEURISTIC_T, EDGE_STORAGE_T>::extractPath(const EnumeratedNode& goal_node, PathSolution<typename SEARCH_PROBLEM_T::node_t, EDGE_STORAGE_T, COST_VECTOR_T>& path_solution, const PathEnumeratedNodeMap& node_map) {
        EnumeratedNode curr_enum_node = goal_node;
        path_solution.node_path.emplace_back(node_map.getNode(curr_enum_node));

        while (!node_map.isInit(curr_enum_node)) {
            // Push back edge
            path_solution.edge_path.emplace_back(node_map.getParentEdge(curr_enum_node));

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