#pragma once

#include <functional>
#include <map>
#include <set>
#include <memory>
#include <unordered_map>

#include "core/Graph.h"

#include "graph_search/SearchProblem.h"

namespace TP {
namespace GraphSearch {

    template <class SYMBOLIC_GRAPH_T, class COST_T, SearchDirection SEARCH_DIRECTION, class HEURISTIC_T = ZeroHeuristic<Node, COST_T>>
    struct SymbolicSearchProblem {
        public:
            typedef 
        public: // Methods & members required by any search problem
            
            // Extension methods
            inline std::vector<Node> neighbors(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getChildren(node);
                else 
                    return m_graph->getParents(node);
            }

            inline std::vector<typename SYMBOLIC_GRAPH_T::edge_t> neighborEdges(Node node) const {
                if constexpr (SEARCH_DIRECTION == SearchDirection::Forward)
                    return m_graph->getOutgoingEdges(node);
                else
                    return m_graph->getIncomingEdges(node);
            }

            // Termination goal function
            virtual inline bool goal(const Node& node) const {return m_goal_node_set.contains(node);}

            // Quantative methods
            inline COST_T gScore(const COST_T& parent_g_score, const SYMBOLIC_GRAPH_T::edge_t& edge) const {return parent_g_score + m_edgeToCost(edge);}
            COST_T hScore(const Node& node) const {return heuristic.operator()(node);}

            // Member variables
            std::vector<Node> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            typedef COST_T(*edgeToCostFunction)(const EDGE_T&);

            QuantitativeGraphSearchProblem(const std::shared_ptr<SYMBOLIC_GRAPH_T>& graph, const std::vector<Node>& initial_node_set_, const std::set<Node>& goal_node_set, edgeToCostFunction edgeToCost) 
                : initial_node_set(initial_node_set_) 
                , m_graph(graph)
                , m_goal_node_set(goal_node_set)
                , m_edgeToCost(edgeToCost)
                {}

        private:
            const std::shared_ptr<SYMBOLIC_GRAPH_T> m_graph;
            std::set<Node> m_goal_node_set;
            edgeToCostFunction m_edgeToCost;

    };


    // Single-Objective tools

    template <class NODE_T, class EDGE_STORAGE_T>
    struct Connection {
        Connection() = delete;
        Connection(const NODE_T& node_, const EDGE_STORAGE_T& edge_) : node(node_), edge(edge_) {}
        Connection(const Connection& other) : node(other.node), edge(other.edge) {}
        NODE_T node;
        EDGE_STORAGE_T edge;
    };

    template <class NODE_T, class EDGE_STORAGE_T, class COST_T>
    struct PathSolution {
        PathSolution() = default;
        PathSolution(PathSolution&& other) : node_path(std::move(other.node_path)), edge_path(std::move(other.edge_path)), path_cost(std::move(other.path_cost)) {}
        std::vector<NODE_T> node_path;
        std::vector<EDGE_STORAGE_T> edge_path;
        COST_T path_cost = COST_T{};
    };

    template <class NODE_T, class EDGE_STORAGE_T>
    using SearchTree = std::map<NODE_T, Connection<NODE_T, EDGE_STORAGE_T>>;
    
    template <class NODE_T, class COST_T>
    using MinCostMap = std::map<NODE_T, COST_T>;

    template <class NODE_T, class EDGE_STORAGE_T, class COST_T>
    struct SingleObjectiveSearchResult {
        public:
            SingleObjectiveSearchResult(bool retain_search_tree = true, bool retain_min_cost_map = true)
                : search_tree(std::make_shared<SearchTree<NODE_T, EDGE_STORAGE_T>>())
                , min_cost_map(std::make_shared<MinCostMap<NODE_T, COST_T>>()) 
                , m_retain_search_tree(retain_search_tree)
                , m_retain_min_cost_map(retain_min_cost_map)
                {}

            bool success = false;
            PathSolution<NODE_T, EDGE_STORAGE_T, COST_T> solution;
            std::shared_ptr<SearchTree<NODE_T, EDGE_STORAGE_T>> search_tree;
            std::shared_ptr<MinCostMap<NODE_T, COST_T>> min_cost_map;

            void package() { // Free the memory of the search tree and min cost map if the user desires
                if (!m_retain_search_tree) search_tree.reset();
                if (!m_retain_min_cost_map) min_cost_map.reset();
            }
        private:
            bool m_retain_search_tree, m_retain_min_cost_map;
    };

}
}