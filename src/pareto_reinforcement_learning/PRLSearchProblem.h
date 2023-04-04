#pragma once

#include "TaskPlanner.h"

#include "BehaviorHandler.h"
#include "HistoryNode.h"

namespace PRL {

template <class BEHAVIOR_HANDLER_T>
struct PRLSearchProblem {
    public: // Dependent types required by any search problem
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

        typedef SymbolicProductGraph graph_t;
        typedef StepHistoryNode<SymbolicProductGraph::node_t> node_t;
        typedef BEHAVIOR_HANDLER_T::CostVector cost_t;
        typedef SymbolicProductGraph::edge_t edge_t;

    public: // Methods & members required by any search problem
        
        // Extension methods (increment the history node)
        inline const std::vector<node_t> neighbors(const node_t& node) const {
            std::vector<SymbolicProductGraph::node_t> children = m_graph->getChildren(node.base_node);
            std::vector<node_t> history_nodes(children.size);
            for (uint32_t i=0; i<children.size(); ++i) {
                history_nodes[i].base_node = children[i];
                history_nodes[i].step = node.step + 1;
            }
            return history_nodes;
        }

        inline const std::vector<edge_t> neighborEdges(const node_t& node) const {
            return m_graph->getOutgoingEdges(node.base);
        }

        // Termination goal node (terminate at the step horizon)
        inline bool goal(const node_t& node) const {return node.step == m_step_horizon;}

        // Quantative methods
        inline cost_t gScore(const node_t& node, const cost_t& parent_g_score, const edge_t& edge) const {
            return parent_g_score + m_behavior_handler(node.base_node, edge.action).getCostVector();
        }
        inline cost_t hScore(const node_t& node) const {return cost_t{};}

        // Member variables
        std::set<node_t> initial_node_set;
        //HEURISTIC_T heuristic = HEURISTIC_T{}; // assumes default ctor

    public:
        PRLSearchProblem(const std::shared_ptr<SymbolicProductGraph>& product, uint32_t step_horizon, const std::shared_ptr<BEHAVIOR_HANDLER_T>& behavior_handler)
            : m_product(product)
            , m_behavior_handler(behavior_handler)
            , m_step_horizon(step_horizon)
            {}

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;
        const uint32_t m_step_horizon;
};
}