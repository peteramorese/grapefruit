#pragma once

#include <algorithm>

#include "planners/PreferenceCostObjective.h"

#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"


namespace GF {
namespace Planner {

    // Simple cumulative action cost
    template<class SYMBOLIC_GRAPH_T, class COLLAPSED_COST_T> 
    struct CostObjective : public PreferenceCostObjective<COLLAPSED_COST_T> {

        CostObjective() = default;
        CostObjective(const CostObjective&) = default;
        CostObjective(CostObjective&&) = default;
        CostObjective(const SYMBOLIC_GRAPH_T& graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) {
            cost = static_cast<COLLAPSED_COST_T>(edge);
        }

        COLLAPSED_COST_T cost = COLLAPSED_COST_T{};

        bool operator<(const CostObjective& other) const {return (other > REQ_TOLERANCE) ? cost < other.cost - REQ_TOLERANCE : false;}
        bool operator==(const CostObjective& other) const {return diff(cost, other.cost) < REQ_TOLERANCE;}
        void operator+=(const CostObjective& other) {cost += other.cost;}
        void operator=(const CostObjective& other) {cost = other.cost;}

        operator COLLAPSED_COST_T() const {return cost;}
    };

    // Minimize the sum of the delay such that the tasks are completed 'more' in order
    template <class SYMBOLIC_GRAPH_T, class COLLAPSED_COST_T>
    struct SumDelayPreferenceCostObjective : public PreferenceCostSet<COLLAPSED_COST_T> {

        SumDelayPreferenceCostObjective() 
            : PreferenceCostSet<COLLAPSED_COST_T>()  {}
        SumDelayPreferenceCostObjective(const SumDelayPreferenceCostObjective&) = default;
        SumDelayPreferenceCostObjective(const SYMBOLIC_GRAPH_T& sym_graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
            : PreferenceCostSet<COLLAPSED_COST_T>(sym_graph.rank() - 1) 
        {
            auto unwrapped_node = sym_graph.getUnwrappedNode(node);
            const auto& automata = sym_graph.extractAutomata();
            for (uint32_t i=0; i<unwrapped_node.automata_nodes.size(); ++i) {
                Node automaton_node = unwrapped_node.automata_nodes[i];
                
                // Assign the edge cost if the automaton is not accepting
                this->m_pcs[i] = (!automata[i]->isAccepting(automaton_node)) ? static_cast<COLLAPSED_COST_T>(edge) : COLLAPSED_COST_T{};
            }
        }

        // Convert
        virtual COLLAPSED_COST_T preferenceFunction() const override {
            Containers::SizedArray sorted_pcs = this->m_pcs;
            std::sort(sorted_pcs.begin(), sorted_pcs.end());

            COLLAPSED_COST_T sum_delay = COLLAPSED_COST_T{};
            for (uint32_t i=0; i<this->m_pcs.size(); ++i) sum_delay += (this->m_pcs[i] > sorted_pcs[i]) ? this->m_pcs[i] - sorted_pcs[i] : COLLAPSED_COST_T{};
            
            return sum_delay;
        }

        void printPCS() const {
            LOG("Printing PCS");
            for (uint32_t i=0; i<this->m_pcs.size(); ++i) {
                LOG("Task " << i <<": " << this->m_pcs[i]);
            }
        }
    };

    
    enum class CostInheritor {
        Model,
        Automata
    };

    // Minimize the cumulative weighted sum of automata costs
    template <class SYMBOLIC_GRAPH_T, class COLLAPSED_COST_T, CostInheritor COST_INHERITOR_T = CostInheritor::Automata>
    struct WeightedSumPreferenceCostObjective : public PreferenceCostSet<COLLAPSED_COST_T> {
        public:
            WeightedSumPreferenceCostObjective() 
                : PreferenceCostSet<COLLAPSED_COST_T>()  {}
            WeightedSumPreferenceCostObjective(const WeightedSumPreferenceCostObjective&) = default;
            WeightedSumPreferenceCostObjective(const SYMBOLIC_GRAPH_T& sym_graph, const SYMBOLIC_GRAPH_T::node_t& node, SYMBOLIC_GRAPH_T::edge_t&& edge) 
                : PreferenceCostSet<COLLAPSED_COST_T>(sym_graph.rank() - 1) 
            {
                ASSERT(getWeights().size() == this->m_pcs.size(), "Number of set weights is not equal to the number of tasks");
                auto unwrapped_node = sym_graph.getUnwrappedNode(node);
                const auto& automata = sym_graph.extractAutomata();
                for (uint32_t i=0; i<unwrapped_node.automata_nodes.size(); ++i) {
                    Node automaton_node = unwrapped_node.automata_nodes[i];
                    
                    // Assign the edge cost if the automaton is not accepting
                    if constexpr (COST_INHERITOR_T == CostInheritor::Automata) {
                        this->m_pcs[i] = (!automata[i]->isAccepting(automaton_node)) ? edge.getAutomatonCost()[i] : COLLAPSED_COST_T{};
                    } else {
                        this->m_pcs[i] = (!automata[i]->isAccepting(automaton_node)) ? static_cast<COLLAPSED_COST_T>(edge) : COLLAPSED_COST_T{};
                    }
                }
            }

            // Convert
            virtual COLLAPSED_COST_T preferenceFunction() const override {
                COLLAPSED_COST_T weighted_sum = COLLAPSED_COST_T{};

                const auto& weights = getWeights();

                for (uint32_t i=0; i<this->m_pcs.size(); ++i) weighted_sum += weights[i] * this->m_pcs[i];
                
                return weighted_sum;
            }

        public:
            static void setWeights(const std::vector<COLLAPSED_COST_T>& weights) {getWeights() = weights;}

        private:
            inline static std::vector<COLLAPSED_COST_T>& getWeights() {static std::vector<COLLAPSED_COST_T> s_weights(0); return s_weights;}
            
    };

}
}