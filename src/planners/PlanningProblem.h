#pragma once

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Graph.h"

#include "graph_search/SearchProblem.h"


namespace TP {
namespace Planner {

    struct Plan {
        using SymbolicProductGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::DFA, DiscreteModel::ModelEdgeInheritor<DiscreteModel::TransitionSystem, FormalMethods::DFA>>;

        Plan(const GraphSearch::PathSolution<Node, DiscreteModel::TransitionSystemLabel, DiscreteModel::TransitionSystemLabel::cost_t>& path, std::shared_ptr<SymbolicProductGraph> sym_graph, bool success) 
            : success(success)
            , product_node_sequence(path.node_path)
        {
            ts_node_sequence.reserve(product_node_sequence.size());
            state_sequence.reserve(product_node_sequence.size());
            for (auto prod_node : product_node_sequence) {
                Node ts_node = sym_graph->getUnwrappedNode(prod_node).ts_node;
                ts_node_sequence.push_back(ts_node);
                state_sequence.push_back(sym_graph->getModel().getGenericNodeContainer()[ts_node]);
            }

            action_sequence.reserve(path.edge_path.size());
            for (auto edge : path.edge_path) action_sequence.push_back(edge.action);
        }


        bool success;
        std::vector<Node> product_node_sequence;
        std::vector<Node> ts_node_sequence;
        std::vector<DiscreteModel::State> state_sequence;
        std::vector<DiscreteModel::Action> action_sequence;

        void print() const {
            LOG("Priting plan");
            
            // Align text:
            uint32_t max_action_str_len = 0;
            for (const auto& a : action_sequence) {
                if (a.size() > max_action_str_len) max_action_str_len = a.size();
            }
            max_action_str_len += 2;

            for (uint32_t i=0; i<ts_node_sequence.size(); ++i) {
                std::string s = "Node: " +std::to_string(ts_node_sequence[i]) + " [" + state_sequence[i].to_str() + "]";
                if (i != 0) {
                    std::string a = action_sequence[i-1] + " -> ";
                    std::string a_adj(max_action_str_len + 4 - a.size(), ' ');
                    PRINT_NAMED(a_adj + a, s);
                } else {
                    PRINT_NAMED(std::string(max_action_str_len + 4, ' '), s);
                }

            }
        }
    };
}
}