#pragma once

#include <algorithm>

#include "planners/PreferenceCostObjective.h"

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"


namespace TP {
namespace Planner {


    template <class SYMBOLIC_GRAPH_T, class INHERITED_COST_T>
    struct SumDelayPreferenceCostObjective : public PreferenceCostSet<INHERITED_COST_T> {

        SumDelayPreferenceCostObjective(const SYMBOLIC_GRAPH_T& graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
            : PreferenceCostSet<INHERITED_COST_T>(sym_graph.rank() - 1) 
        {
            auto unwrapped_node = graph->getUnwrappedNode(node);
            const auto& automata = graph->extractAutomata();
            for (uint32_t i=0; i<unwrapped_node.automata_nodes.size(); ++i) {
                Node automaton_node = unwrapped_node.automata_nodes[i];
                
                // Assign the edge cost if the automaton is not accepting
                m_pcs[i] = (!automata[i]->isAccepting(automaton_node)) ? edge.toCost() : INHERITED_COST_T{};
            }
        }

        // Convert
        virtual INHERITED_COST_T preferenceFunction() const override {
            Containers::SizedArray sorted_pcs = m_pcs;
            std::sort(sorted_pcs.begin(), sorted_pcs.end());

            INHERITED_COST_T sum_delay = INHERITED_COST_T{};
            for (uint32_t i=0; i<m_pcs.size(); ++i) sum_delay += (m_pcs[i] > sorted_pcs[i]) ? m_pcs[i] - sorted_pcs[i] : INHERITED_COST_T{};

            return sum_delay;
        }
    };

    template<class SYMBOLIC_GRAPH_T, class INHERITED_COST_T> 
    struct CostPreferenceObjective {
        CostPreferenceObjective(const SYMBOLIC_GRAPH_T& graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) {
            cost = edge.toCost();
        }

        INHERITED_COST_T cost;
        opeartor INHERITED_COST_T() const {return cost;}
    };

}
}