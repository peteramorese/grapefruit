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
        SumDelayPreferenceCostObjective() 
            : PreferenceCostSet<INHERITED_COST_T>()  {}
        SumDelayPreferenceCostObjective(const SumDelayPreferenceCostObjective&) = default;
        SumDelayPreferenceCostObjective(const SYMBOLIC_GRAPH_T& sym_graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
            : PreferenceCostSet<INHERITED_COST_T>(sym_graph.rank() - 1) 
        {
            auto unwrapped_node = sym_graph.getUnwrappedNode(node);
            const auto& automata = sym_graph.extractAutomata();
            for (uint32_t i=0; i<unwrapped_node.automata_nodes.size(); ++i) {
                Node automaton_node = unwrapped_node.automata_nodes[i];
                
                // Assign the edge cost if the automaton is not accepting
                this->m_pcs[i] = (!automata[i]->isAccepting(automaton_node)) ? static_cast<INHERITED_COST_T>(edge) : INHERITED_COST_T{};
            }
        }

        // Convert
        virtual INHERITED_COST_T preferenceFunction() const override {
            Containers::SizedArray sorted_pcs = this->m_pcs;
            std::sort(sorted_pcs.begin(), sorted_pcs.end());
            //
            //LOG("pcs size: " << this->m_pcs.size());
            //for (uint32_t i=0; i<this->m_pcs.size(); ++i) LOG("pcs[i]: " << this->m_pcs[i] << " sorted pcs[i]: " <<  sorted_pcs[i]);
            //

            INHERITED_COST_T sum_delay = INHERITED_COST_T{};
            for (uint32_t i=0; i<this->m_pcs.size(); ++i) sum_delay += (this->m_pcs[i] > sorted_pcs[i]) ? this->m_pcs[i] - sorted_pcs[i] : INHERITED_COST_T{};
            
            return sum_delay;
        }
    };

    template<class SYMBOLIC_GRAPH_T, class INHERITED_COST_T> 
    struct CostObjective {
        CostObjective() = default;
        CostObjective(const CostObjective&) = default;
        CostObjective(CostObjective&&) = default;
        CostObjective(const SYMBOLIC_GRAPH_T& graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) {
            cost = static_cast<INHERITED_COST_T>(edge);
        }

        INHERITED_COST_T cost = INHERITED_COST_T{};

        bool operator<(const CostObjective& other) const {return cost < other.cost;}
        void operator+=(const CostObjective& other) {cost += other.cost;}
        void operator=(const CostObjective& other) {cost = other.cost;}

        operator INHERITED_COST_T() {return cost;}
        operator const INHERITED_COST_T&() const {return cost;}
    };

}
}