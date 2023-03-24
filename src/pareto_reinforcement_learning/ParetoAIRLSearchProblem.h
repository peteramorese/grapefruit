#pragma once

#include "TaskPlanner.h"

#include "BehaviorHandler.h"

namespace PRL {

template <class EXPLICIT_GRAPH_T, class BEHAVIOR_HANDLER_T>
struct PRLSearchProblem {
    public: // Dependent types required by any search problem

        typedef EXPLICIT_GRAPH_T graph_t;
        typedef EXPLICIT_GRAPH_T::node_t node_t;
        typedef BEHAVIOR_HANDLER_T::CostVector cost_t;
        typedef EXPLICIT_GRAPH_T::edge_t edge_t;

    public: // Methods & members required by any search problem
        
        // Extension methods
        inline const std::vector<node_t>& neighbors(node_t node) const {
            return m_graph->getChildren(node);
        }

        inline const std::vector<edge_t>& neighborEdges(node_t node) const {
            return m_graph->getOutgoingEdges(node);
        }

        // Termination goal node
        inline bool goal(const node_t& node) const {return m_goal_node_set.contains(node);}

        // Quantative methods
        inline cost_t gScore(const cost_t& parent_g_score, const edge_t& edge) const {return parent_g_score + m_edgeToCostVector(edge);}
        cost_t hScore(const node_t& node) const {return cost_t{};}

        // Member variables
        std::set<node_t> initial_node_set;
        //HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

    public:
        //typedef cost_t(*edgeToCostVectorFunction)(const edge_t&);

        PRLSearchProblem(const std::shared_ptr<EXPLICIT_GRAPH_T>& graph, const std::shared_ptr<const BEHAVIOR_HANDLER_T>& behavior_handler, const std::set<Node> initial_node_set_, const std::set<Node>& goal_node_set, edgeToCostVectorFunction edgeToCostVector) 
            : initial_node_set(initial_node_set_) 
            , m_graph(graph)
            , m_behavior_handler(behavior_handler)
            {}

    private:
        std::shared_ptr<const EXPLICIT_GRAPH_T> m_graph;
        std::shared_ptr<const BEHAVIOR_HANDLER_T> m_behavior_handler;
        //std::set<Node> m_goal_node_set;
        //edgeToCostVectorFunction m_edgeToCostVector;

};
}