#pragma once

#include "TaskPlanner.h"

#include "BehaviorHandler.h"

namespace PRL {

template <class BEHAVIOR_HANDLER_T>
struct PRLSearchProblem {
    public: // Dependent types required by any search problem
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        typedef SymbolicProductGraph graph_t;
        typedef SymbolicProductGraph::node_t node_t;
        typedef BEHAVIOR_HANDLER_T::CostVector cost_t;
        typedef SymbolicProductGraph::edge_t edge_t;

    public: // Methods & members required by any search problem
        
        // Extension methods
        inline const std::vector<node_t> neighbors(node_t node) const {
            return m_graph->getChildren(node);
        }

        inline const std::vector<edge_t> neighborEdges(node_t node) const {
            return m_graph->getOutgoingEdges(node);
        }

        // Termination goal node
        inline bool goal(const node_t& node) const {return m_goal_node_set.contains(node);}

        // Quantative methods
        inline cost_t gScore(node_t node, const cost_t& parent_g_score, const edge_t& edge) const {
            return parent_g_score + m_behavior_handler(node, edge.action).getCostVector();
        }
        cost_t hScore(const node_t& node) const {return cost_t{};}

        // Member variables
        std::set<node_t> initial_node_set;
        //HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

    public:
        //typedef cost_t(*edgeToCostVectorFunction)(const edge_t&);
        PRLSearchProblem(const std::shared_ptr<TransitionSystem>& ts, const std::vector<std::shared_ptr<DFA>>& automata)
            : m_product(std::make_shared<SymbolicProductGraph>(ts, automata))
            , m_behavior_handler(std::make_shared<BEHAVIOR_HANDLER_T>(ts))
            {}

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;
        //std::set<Node> m_goal_node_set;
        //edgeToCostVectorFunction m_edgeToCostVector;
};
}