#pragma once

#include <map>
#include <set>
#include <memory>
#include <unordered_map>

#include "core/Graph.h"

#include "graph_search/SearchProblem.h"

namespace TP {
namespace GraphSearch {

    template <class SYMBOLIC_GRAPH_T, class COST_T, SearchDirection SEARCH_DIRECTION, class HEURISTIC_T = ZeroHeuristic<Node, COST_T>>
    struct QuantitativeSymbolicSearchProblem {
        public: // Methods & members required by any search problem
            
            // Extension methods
            inline std::vector<typename SYMBOLIC_GRAPH_T::node_t> neighbors(Node node) const {
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
            virtual bool goal(const Node& node) const {return m_goal_node_set.contains(node);}

            // Quantative methods
            inline COST_T gScore(const Node& src_node, const Node& dst_node, const COST_T& parent_g_score, const SYMBOLIC_GRAPH_T::edge_t& edge) const {return parent_g_score + static_cast<COST_T>(edge);}
            COST_T hScore(const SYMBOLIC_GRAPH_T::node_t& node) const {return heuristic.operator()(node);}

            // Member variables
            std::vector<typename SYMBOLIC_GRAPH_T::node_t> initial_node_set;
            HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

        public:
            //typedef COST_T(*edgeToCostFunction)(const SYMBOLIC_GRAPH_T::edge_t&);

            QuantitativeSymbolicSearchProblem(const std::shared_ptr<SYMBOLIC_GRAPH_T>& graph, const std::vector<typename SYMBOLIC_GRAPH_T::node_t>& initial_node_set_, const std::set<typename SYMBOLIC_GRAPH_T::node_t>& goal_node_set) 
                : initial_node_set(initial_node_set_) 
                , m_graph(graph)
                , m_goal_node_set(goal_node_set)
                //, m_edgeToCost(edgeToCost)
                {}

        protected:
            const std::shared_ptr<SYMBOLIC_GRAPH_T> m_graph;
            std::set<typename SYMBOLIC_GRAPH_T::node_t> m_goal_node_set;
            //edgeToCostFunction m_edgeToCost;

    };

}
}