#pragma once

#include "planners/PreferenceCostObjective.h"

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"


namespace TP {
namespace Planner {


    template <class SYMBOLIC_GRAPH_T, class INHERITED_COST_T>
    struct OrderedPreferenceCostObjective : public PreferenceCostSet<INHERITED_COST_T> {

        OrderedPreferenceCostObjective(const SYMBOLIC_GRAPH_T& graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
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
        virtual DiscreteModel::TransitionSystemLabel::cost_t preferenceFunction() const override {
        }
    };

}
}