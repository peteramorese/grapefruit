#pragma once

#include "Grapefruit.h"

#include "BehaviorHandler.h"
#include "HistoryNode.h"

namespace PRL {

template <uint64_t N, class BEHAVIOR_HANDLER_T>
struct SearchProblem {
    public: // Dependent types required by any search problem
        using SymbolicProductGraph = GF::DiscreteModel::SymbolicProductAutomaton<
            GF::DiscreteModel::TransitionSystem, 
            GF::FormalMethods::DFA, 
            GF::DiscreteModel::ModelEdgeInheritor<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA>>;

        typedef SymbolicProductGraph graph_t;
        typedef TaskHistoryNode<SymbolicProductGraph::node_t> node_t;
        typedef GF::Containers::FixedArray<N, float> cost_t;
        typedef SymbolicProductGraph::edge_t edge_t;
        typedef GF::DiscreteModel::Action action_t;

    public: // Methods & members required by any search problem
        
        // Extension methods (increment the history node)
        inline const std::vector<node_t> neighbors(const node_t& node) const {
            std::vector<SymbolicProductGraph::node_t> children = m_product->children(node.base_node);
            std::vector<node_t> history_nodes;
            history_nodes.reserve(children.size());
            for (uint32_t i=0; i<children.size(); ++i) {
                uint8_t n_tasks_completed = 0;
                for (GF::DiscreteModel::ProductRank automaton_i = 0; automaton_i < m_product->rank() - 1; ++automaton_i) {
                    if (!m_product->acc(node.base_node, automaton_i) && m_product->acc(children[i], automaton_i)) {
                        ++n_tasks_completed;
                    }
                }
                history_nodes.emplace_back(children[i], node.n_completed_tasks + n_tasks_completed);
            }
            return history_nodes;
        }

        inline const std::vector<edge_t> neighborEdges(const node_t& node) const {
            return m_product->outgoingEdges(node.base_node);
        }

        // Termination goal node (terminate at the step horizon)
        inline bool goal(const node_t& node) const {
            return node.n_completed_tasks >= m_completed_tasks_horizon;
        }

        // Quantative methods
        inline cost_t gScore(const node_t& src_node, const node_t& dst_node, const cost_t& parent_g_score, const edge_t& edge) const {
            return parent_g_score + m_behavior_handler->getCostVector(src_node, dst_node, edge.action);
        }
        inline cost_t hScore(const node_t& node) const {return cost_t{};}

        // Member variables
        std::set<node_t> initial_node_set;

    public:
        SearchProblem(const std::shared_ptr<SymbolicProductGraph>& product, SymbolicProductGraph::node_t init_node, uint8_t completed_tasks_horizon, const std::shared_ptr<BEHAVIOR_HANDLER_T>& behavior_handler)
            : m_product(product)
            , m_behavior_handler(behavior_handler)
            , m_completed_tasks_horizon(completed_tasks_horizon)
        {
            // Initialize to no completed tasks
            initial_node_set = {TaskHistoryNode<SymbolicProductGraph::node_t>(init_node, 0)};
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::shared_ptr<BEHAVIOR_HANDLER_T> m_behavior_handler;
        const uint8_t m_completed_tasks_horizon;
};
}